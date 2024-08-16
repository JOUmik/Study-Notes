





# C#构建部分

## Recast

[Recast-Detour看这里就够了！_recast算法-CSDN博客](https://blog.csdn.net/romantic_jie/article/details/128791513)

整体实现逻辑基于recast，可以先看一下



## 主体函数

~~~c#
bool BuildMesh(Voxelize vox, int tileSizeX, int tileSizeZ, bool bExcavation = false)
{
    bool result = false;

    //在世界中tile的大小
    float tcsx = tileSizeX * cellSize;
    float tcsz = tileSizeZ * cellSize;

    int voxelCharacterRadius = Mathf.CeilToInt(characterRadius / cellSize);

    Vector3 forcedBoundsMin = forcedBounds.min;
    Vector3 forcedBoundsMax = forcedBounds.max;

    Bounds bounds = new Bounds();
    bounds.SetMinMax(forcedBoundsMin,
        new Vector3(tcsx + forcedBoundsMin.x, forcedBoundsMax.y, tcsz + forcedBoundsMin.z));

    //在每边扩展bordersize大小的体素
    bounds.Expand(new Vector3(vox.borderSize, 0, vox.borderSize) * cellSize * 2);

    vox.forcedBounds = bounds;
    vox.width = tileSizeX + vox.borderSize * 2;
    vox.depth = tileSizeZ + vox.borderSize * 2;
    vox.relevantGraphSurfaceMode = RelevantGraphSurfaceMode.DoNotRequire;
    vox.minRegionSize = Mathf.RoundToInt(minRegionSize / (cellSize * cellSize)); //换算为体素的单位，vox.minRegionSize代表最小体素区域面积大小

    //Init voxelizer
    vox.Init();
    vox.CollectMeshes(); // 收集所有用于烘培的mesh
    vox.VoxelizeInput(); // 体素化，生成linkedSpans（表示最原始的空间信息）
    // 根据最大的爬坡高度voxelWalkableClimb去掉太陡峭的坡
    vox.FilterLedges(vox.voxelWalkableHeight, vox.voxelWalkableClimb, vox.cellSize, vox.cellHeight,
        vox.forcedBounds.min);
    // 根据最大可穿越的身高voxelWalkableHeight去掉所有高度不够的区域
    vox.FilterLowHeightSpans(vox.voxelWalkableHeight, vox.cellSize, vox.cellHeight, vox.forcedBounds.min);

    // 生成compactSpans表示网格信息（相同坐标的网格连续存放），及compactCells表示网格索引（index表示索引开始位置，count表示数量）
    vox.BuildCompactField();

    vox.BuildVoxelConnections(); // 生成每个网格与其邻居是否有相邻关系，及相邻的层的索引
    vox.ErodeWalkableArea(voxelCharacterRadius); // 根据角色半径去掉边缘的一圈
    vox.BuildDistanceField(); // 生成每个网格到边缘的最短距离dist
    vox.BuildRegions(); // 对所有的岛屿标上不同的reg（方便识别网格间的连通性）

    VoxelContourSet cset = new VoxelContourSet();

    // 生成所有的边界（conts）上的点（中间的点全都不要，鼓包就会变平台，这是算法缺陷？鼓包问题请中间挖个洞来解决）
    vox.BuildContours(maxError, 1, cset, Voxelize.RC_CONTOUR_TESS_WALL_EDGES);

    VoxelMesh mesh;
    result = vox.BuildPolyMesh(cset, 3, out mesh); // 根据边界生成mesh

    //将顶点正确放置在世界中，这里的顶点每相邻的3个代表一个三角形
    for (int i = 0; i < mesh.verts.Length; i++)
    {
        // VInt3.Precision = 1000, 这里的矩阵运算是标量运算
        mesh.verts[i] = ((mesh.verts[i] * VInt3.Precision) * vox.cellScale) + (VInt3)vox.voxelOffset;

        //Debug.DrawRay (matrix.MultiplyPoint3x4(vertices[i]),Vector3.up,Color.red);
    }

    verts = mesh.verts;
    tris = mesh.tris;
    layerFlags = mesh.layerFlag;
    RemoveOverlappedVerts(ref verts, ref tris, ref layerFlags);

    return result;
}
~~~



## 初始化（Init）

```c#
public void Init(List<GeoRegion> _geoRes = null)
{
    //Initialize the voxel area
    if (voxelArea == null || voxelArea.width != width || voxelArea.depth != depth)
    {
        voxelArea = new VoxelArea(width, depth);
    }
    else
    {
        voxelArea.Reset();
    }

    geoRegions = _geoRes;
}
```



## 收集Mesh（CollectMeshes）

### CollectMeshes

~~~c#
public void CollectMeshes()
{
    CollectMeshes(inputExtraMeshes, forcedBounds, out inputVertices, out inputTriangles);
}

public static void CollectMeshes(List<ExtraMesh> extraMeshes, Bounds bounds, out Vector3[] verts, out int[] tris)
{
    verts = null;
    tris = null;
}
~~~





## 体素化（VoxelizeInput）

### VoxelizeInput

```c#
public void VoxelizeInput()
{
    //Debug.DrawLine (forcedBounds.min,forcedBounds.max,Color.blue);

    Vector3 min = forcedBounds.min;
    voxelOffset = min;

    float ics = 1F / cellSize;
    float ich = 1F / cellHeight;
    float slopeLimit = Mathf.Cos(Mathf.Atan(Mathf.Tan(maxSlope * Mathf.Deg2Rad) * (ich * cellSize)));

    float[] vTris = new float[3 * 3];
    float[] vOut = new float[7 * 3];
    float[] vRow = new float[7 * 3];
    float[] vCellOut = new float[7 * 3];
    float[] vCell = new float[7 * 3];

    if (inputExtraMeshes == null) 
        throw new System.NullReferenceException("inputExtraMeshes not set");

    //Find the largest lenghts of vertex arrays and check for meshes which can be skipped
    int maxVerts = 0;
    for (int m = 0; m < inputExtraMeshes.Count; m++)
    {
        if (!inputExtraMeshes[m].bounds.Intersects(forcedBounds)) 
            continue;

        maxVerts = System.Math.Max(inputExtraMeshes[m].vertices.Length, maxVerts);
    }

    //Create buffer, here vertices will be stored multiplied with the local-to-voxel-space matrix
    Vector3[] verts = new Vector3[maxVerts];

    Matrix4x4 voxelMatrix = Matrix4x4.Scale(new Vector3(ics, ich, ics)) * Matrix4x4.TRS(-min, Quaternion.identity, Vector3.one);

    for (int m = 0; m < inputExtraMeshes.Count; m++)
    {
        ExtraMesh mesh = inputExtraMeshes[m];
        bool isMirror = MMGame_Math.isMirror(mesh.matrix);
        if (isMirror)
        {
            SGameLog.Debug(StringHelper.Format("GameObject {0} is mirrored!", mesh.name));
        }

        if (!mesh.bounds.Intersects(forcedBounds)) 
            continue;

        Matrix4x4 matrix = mesh.matrix;
        matrix = voxelMatrix * matrix;

        Vector3[] vs = mesh.vertices;
        int[] tris = mesh.triangles;
        int trisLength = tris.Length;

        for (int i = 0; i < vs.Length; i++)
        {
            verts[i] = matrix.MultiplyPoint3x4(vs[i]);
        }

        int mesharea = mesh.area;

        for (int i = 0; i < trisLength; i += 3)
        {
            Vector3 p1;
            Vector3 p2;
            Vector3 p3;

            if (isMirror)
            {
                p1 = verts[tris[i]];
                p2 = verts[tris[i + 2]];
                p3 = verts[tris[i + 1]];
            }
            else
            {
                p1 = verts[tris[i]];
                p2 = verts[tris[i + 1]];
                p3 = verts[tris[i + 2]];
            }

            int minX = (int)(Utility.Min(p1.x, p2.x, p3.x));
            int minZ = (int)(Utility.Min(p1.z, p2.z, p3.z));
            int maxX = (int)System.Math.Ceiling(Utility.Max(p1.x, p2.x, p3.x));
            int maxZ = (int)System.Math.Ceiling(Utility.Max(p1.z, p2.z, p3.z));

            minX = Mathf.Clamp(minX, 0, voxelArea.width - 1);
            maxX = Mathf.Clamp(maxX, 0, voxelArea.width - 1);
            minZ = Mathf.Clamp(minZ, 0, voxelArea.depth - 1);
            maxZ = Mathf.Clamp(maxZ, 0, voxelArea.depth - 1);

            if (minX >= voxelArea.width || minZ >= voxelArea.depth || maxX <= 0 || maxZ <= 0) 
                continue;

            //Debug.DrawLine (p1*cellSize+min+Vector3.up*0.2F,p2*cellSize+voxelOffset+Vector3.up*0.1F,Color.red);
            //Debug.DrawLine (p2*cellSize+min+Vector3.up*0.1F,p3*cellSize+voxelOffset,Color.red);

            int area;
            Vector3 normal = Vector3.Cross(p2 - p1, p3 - p1);

            float dot = Vector3.Dot(normal.normalized, Vector3.up);
            if (dot < slopeLimit)
            {
                area = UnwalkableArea;
            }
            else
            {
                // mesharea改为了layerflag
                area = mesharea == 0 ? 1 : mesharea;
            }

            //Debug.DrawRay (((p1+p2+p3)/3.0F)*cellSize+voxelOffset,normal,Color.cyan);

            Utility.CopyVector(vTris, 0, p1);
            Utility.CopyVector(vTris, 3, p2);
            Utility.CopyVector(vTris, 6, p3);

            for (int x = minX; x <= maxX; x++)
            {
                int nrow = Utility.ClipPolygon(vTris, 3, vOut, 1F, -x + 0.5F, 0);
                if (nrow < 3)
                    continue;

                nrow = Utility.ClipPolygon(vOut, nrow, vRow, -1F, x + 0.5F, 0);
                if (nrow < 3)
                    continue;

                float clampZ1 = vRow[2];
                float clampZ2 = vRow[2];
                for (int q = 1; q < nrow; q++)
                {
                    float val = vRow[q * 3 + 2];
                    clampZ1 = System.Math.Min(clampZ1, val);
                    clampZ2 = System.Math.Max(clampZ2, val);
                }

                int clampZ1I = AstarMath.Clamp(MMGame_Math.RoundToInt(clampZ1), 0, voxelArea.depth - 1);
                int clampZ2I = AstarMath.Clamp(MMGame_Math.RoundToInt(clampZ2), 0, voxelArea.depth - 1);

                for (int z = clampZ1I; z <= clampZ2I; z++)
                {
                    int ncell = Utility.ClipPolygon(vRow, nrow, vCellOut, 1F, -z + 0.5F, 2);
                    if (ncell < 3)
                        continue;

                    ncell = Utility.ClipPolygonY(vCellOut, ncell, vCell, -1F, z + 0.5F, 2);
                    if (ncell < 3)
                        continue;

                    float sMin = vCell[1];
                    float sMax = vCell[1];
                    for (int q = 1; q < ncell; q++)
                    {
                        float val = vCell[q * 3 + 1];
                        sMin = System.Math.Min(sMin, val);
                        sMax = System.Math.Max(sMax, val);
                    }

                    int maxi = (int)System.Math.Ceiling(sMax);
                    if (maxi >= 0)
                    {
                        int mini = (int)(sMin + 1);
                        voxelArea.AddLinkedSpan(z * voxelArea.width + x, (mini >= 0 ? (uint)mini : 0), (uint)maxi, area, voxelWalkableClimb);
                    }
                }
            }
        }
    }
}
```



### ClipPolygon

```c#
public static int ClipPolygon(float[] vIn, int n, float[] vOut, float multi, float offset, int axis)
{
    float[] d = clipPolygonCache;
    for (int i = 0; i < n; i++)
    {
        d[i] = multi * vIn[i * 3 + axis] + offset;
    }

    //Number of resulting vertices
    int m = 0;

    for (int i = 0, j = n - 1; i < n; j = i, i++)
    {
        bool prev = d[j] >= 0;
        bool curr = d[i] >= 0;

        if (prev != curr)
        {
            int m3 = m * 3;
            int i3 = i * 3;
            int j3 = j * 3;

            float s = d[j] / (d[j] - d[i]);

            vOut[m3 + 0] = vIn[j3 + 0] + (vIn[i3 + 0] - vIn[j3 + 0]) * s;
            vOut[m3 + 1] = vIn[j3 + 1] + (vIn[i3 + 1] - vIn[j3 + 1]) * s;
            vOut[m3 + 2] = vIn[j3 + 2] + (vIn[i3 + 2] - vIn[j3 + 2]) * s;
            
            m++;
        }

        if (curr)
        {
            int m3 = m * 3;
            int i3 = i * 3;

            vOut[m3 + 0] = vIn[i3 + 0];
            vOut[m3 + 1] = vIn[i3 + 1];
            vOut[m3 + 2] = vIn[i3 + 2];

            m++;
        }
    }

    return m;
}
```

### ClipPolygonY

```c#
public static int ClipPolygonY(float[] vIn, int n, float[] vOut, float multi, float offset, int axis)
{
    float[] d = clipPolygonCache;

    for (int i = 0; i < n; i++)
    {
        d[i] = multi * vIn[i * 3 + axis] + offset;
    }

    //Number of resulting vertices
    int m = 0;

    for (int i = 0, j = n - 1; i < n; j = i, i++)
    {
        bool prev = d[j] >= 0;
        bool curr = d[i] >= 0;

        if (prev != curr)
        {
            vOut[m * 3 + 1] = vIn[j * 3 + 1] + (vIn[i * 3 + 1] - vIn[j * 3 + 1]) * (d[j] / (d[j] - d[i]));

            m++;
        }

        if (curr)
        {
            vOut[m * 3 + 1] = vIn[i * 3 + 1];

            m++;
        }
    }

    return m;
}
```

### AddLinkedSpan

```c#
public void AddLinkedSpan(int index, uint bottom, uint top, int area, int voxelWalkableClimb)
{
    /* Check if the span is valid, otherwise we can replace it with a new (valid) span */
    if (linkedSpans[index].bottom == InvalidSpanValue)
    {
        linkedSpans[index] = new LinkedVoxelSpan(bottom, top, area);
        return;
    }

    int prev = -1;
    int oindex = index;

    while (index != -1)
    {
        if (linkedSpans[index].bottom > top)
        {
            break;
        }
        else if (linkedSpans[index].top < bottom)
        {
            prev = index;
            index = linkedSpans[index].next;
        }
        else
        {
            if (linkedSpans[index].bottom < bottom)
            {
                bottom = linkedSpans[index].bottom;
            }

            if (linkedSpans[index].top > top)
            {
                top = linkedSpans[index].top;
            }

            //1 is flagMergeDistance, when a walkable flag is favored before an unwalkable one
            if (AstarMath.Abs((int)top - (int)linkedSpans[index].top) <= voxelWalkableClimb)
            {
                //area = AstarMath.Max(area, linkedSpans[index].area);
                // 合并区域时，layerflag也要合并
                area = area | linkedSpans[index].area;
            }

            int next = linkedSpans[index].next;
            if (prev != -1)
            {
                linkedSpans[prev].next = next;

                if (removedStackCount == removedStack.Length)
                {
                    int[] st2 = new int[removedStackCount * 4];
                    System.Buffer.BlockCopy(removedStack, 0, st2, 0, removedStackCount * sizeof(int));
                    removedStack = st2;
                }

                removedStack[removedStackCount] = index;
                removedStackCount++;

                index = next;
            }
            else if (next != -1)
            {
                linkedSpans[oindex] = linkedSpans[next];

                if (removedStackCount == removedStack.Length)
                {
                    int[] st2 = new int[removedStackCount * 4];
                    System.Buffer.BlockCopy(removedStack, 0, st2, 0, removedStackCount * sizeof(int));
                    removedStack = st2;
                }

                removedStack[removedStackCount] = next;
                removedStackCount++;

                index = linkedSpans[oindex].next;
            }
            else
            {
                linkedSpans[oindex] = new LinkedVoxelSpan(bottom, top, area);
                return;
            }
        }
    }

    if (linkedSpanCount >= linkedSpans.Length)
    {
        LinkedVoxelSpan[] tmp = linkedSpans;
        int count = linkedSpanCount;
        int popped = removedStackCount;
        linkedSpans = new LinkedVoxelSpan[linkedSpans.Length * 2];
        ResetLinkedVoxelSpans();
        linkedSpanCount = count;
        removedStackCount = popped;
        for (int i = 0; i < linkedSpanCount; i++)
        {
            linkedSpans[i] = tmp[i];
        }

        SGameLog.Debug("Layer estimate too low, doubling size of buffer.\nThis message is harmless.");
    }

    int nextIndex;
    if (removedStackCount > 0)
    {
        removedStackCount--;
        nextIndex = removedStack[removedStackCount];
    }
    else
    {
        nextIndex = linkedSpanCount;
        linkedSpanCount++;
    }

    if (prev != -1)
    {
        linkedSpans[nextIndex] = new LinkedVoxelSpan(bottom, top, area, linkedSpans[prev].next);
        linkedSpans[prev].next = nextIndex;
    }
    else
    {
        linkedSpans[nextIndex] = linkedSpans[oindex];
        linkedSpans[oindex] = new LinkedVoxelSpan(bottom, top, area, nextIndex);
    }
}
```

### ResetLinkedVoxelSpans

```c#
private void ResetLinkedVoxelSpans()
{
    int len = linkedSpans.Length;
    linkedSpanCount = width * depth;
    LinkedVoxelSpan df = new LinkedVoxelSpan(InvalidSpanValue, InvalidSpanValue, -1, -1);
    for (int i = 0; i < len;)
    {
        // 16x unrolling, actually improves performance
        linkedSpans[i] = df;
        i++;
        linkedSpans[i] = df;
        i++;
        linkedSpans[i] = df;
        i++;
        linkedSpans[i] = df;
        i++;
        linkedSpans[i] = df;
        i++;
        linkedSpans[i] = df;
        i++;
        linkedSpans[i] = df;
        i++;
        linkedSpans[i] = df;
        i++;
        linkedSpans[i] = df;
        i++;
        linkedSpans[i] = df;
        i++;
        linkedSpans[i] = df;
        i++;
        linkedSpans[i] = df;
        i++;
        linkedSpans[i] = df;
        i++;
        linkedSpans[i] = df;
        i++;
        linkedSpans[i] = df;
        i++;
        linkedSpans[i] = df;
        i++;
    }

    removedStackCount = 0;
}
```

## 过滤太陡峭区域（FilterLedges）

```c#
public void FilterLedges(uint voxelWalkableHeight, int voxelWalkableClimb, float cs, float ch, Vector3 min)
{
    int wd = voxelArea.width * voxelArea.depth;

    #if ASTAR_RECAST_ARRAY_BASED_LINKED_LIST
        LinkedVoxelSpan[] spans = voxelArea.linkedSpans;
    #endif
        int[] DirectionX = voxelArea.DirectionX;
    int[] DirectionZ = voxelArea.DirectionZ;

    int width = voxelArea.width;

    //Filter all ledges
    for (int z = 0, pz = 0; z < wd; z += width, pz++)
    {
        for (int x = 0; x < width; x++)
        {
            if (spans[x + z].bottom == VoxelArea.InvalidSpanValue) 
                continue;

            for (int s = x + z; s != -1; s = spans[s].next)
            {
                //跳过不可行走区域
                if (spans[s].area == UnwalkableArea)
                    continue;

                int bottom = (int)spans[s].top;
                int top = spans[s].next != -1 ? (int)spans[spans[s].next].bottom : VoxelArea.MaxHeightInt;

                int minHeight = VoxelArea.MaxHeightInt;

                int aMinHeight = (int)spans[s].top;
                int aMaxHeight = aMinHeight;

                for (int d = 0; d < 4; d++)
                {
                    int nx = x + DirectionX[d];
                    int nz = z + DirectionZ[d];

                    //Skip out-of-bounds points
                    if (nx < 0 || nz < 0 || nz >= wd || nx >= width)
                    {
                        spans[s].area = UnwalkableArea;
                        break;
                    }

                    int nsx = nx + nz;

                    int nbottom = -voxelWalkableClimb;

                    int ntop = spans[nsx].bottom != VoxelArea.InvalidSpanValue ? (int)spans[nsx].bottom : VoxelArea.MaxHeightInt;

                    if (System.Math.Min(top, ntop) - System.Math.Max(bottom, nbottom) > voxelWalkableHeight)
                    {
                        minHeight = System.Math.Min(minHeight, nbottom - bottom);
                    }

                    //Loop through spans
                    if (spans[nsx].bottom != VoxelArea.InvalidSpanValue)
                    {
                        for (int ns = nsx; ns != -1; ns = spans[ns].next)
                        {
                            nbottom = (int)spans[ns].top;
                            ntop = spans[ns].next != -1 ? (int)spans[spans[ns].next].bottom : VoxelArea.MaxHeightInt;

                            if (System.Math.Min(top, ntop) - System.Math.Max(bottom, nbottom) > voxelWalkableHeight)
                            {
                                minHeight = AstarMath.Min(minHeight, nbottom - bottom);

                                if (System.Math.Abs(nbottom - bottom) <= voxelWalkableClimb)
                                {
                                    if (nbottom < aMinHeight)
                                    {
                                        aMinHeight = nbottom;
                                    }
                                    if (nbottom > aMaxHeight)
                                    {
                                        aMaxHeight = nbottom;
                                    }
                                }
                            }
                        }
                    }
                }

                if (minHeight < -voxelWalkableClimb || (aMaxHeight - aMinHeight) > voxelWalkableClimb)
                {
                    spans[s].area = UnwalkableArea;
                }
            }
        }
    }
}
```



## 过滤过矮区域（FilterLowHeightSpans）

```c#
public void FilterLowHeightSpans(uint voxelWalkableHeight, float cs, float ch, Vector3 min)
{
    int wd = voxelArea.width * voxelArea.depth;

    //Filter all ledges
    LinkedVoxelSpan[] spans = voxelArea.linkedSpans;
    for (int z = 0, pz = 0; z < wd; z += voxelArea.width, pz++)
    {
        for (int x = 0; x < voxelArea.width; x++)
        {
            for (int s = z + x; s != -1 && spans[s].bottom != VoxelArea.InvalidSpanValue; s = spans[s].next)
            {
                uint bottom = spans[s].top;
                uint top = spans[s].next != -1 ? spans[spans[s].next].bottom : VoxelArea.MaxHeight;

                if (top - bottom < voxelWalkableHeight)
                {
                    // 优化区域时，layerflag也要合并
                    if (spans[s].next != -1 && spans[spans[s].next].area > 0)
                        spans[spans[s].next].area |= spans[s].area;
                    spans[s].area = UnwalkableArea;
                }
            }
        }
    }
}
```



## 构建CompactField（BuildCompactField）

### BuildCompactField

```c#
public void BuildCompactField()
{
    int spanCount = voxelArea.GetSpanCount();

    voxelArea.compactSpanCount = spanCount;
    if (voxelArea.compactSpans == null || voxelArea.compactSpans.Length < spanCount)
    {
        voxelArea.compactSpans = new CompactVoxelSpan[spanCount];
        voxelArea.areaTypes = new int[spanCount];
    }

    uint idx = 0;

    int w = voxelArea.width;
    int d = voxelArea.depth;
    int wd = w * d;

    if (voxelWalkableHeight >= 0xFFFF)
    {
        SGameLog.Warning("Too high walkable height to guarantee correctness. Increase voxel height or lower walkable height.");
    }

    LinkedVoxelSpan[] spans = voxelArea.linkedSpans;

    for (int z = 0, pz = 0; z < wd; z += w, pz++)
    {
        for (int x = 0; x < w; x++)
        {
            int spanIndex = x + z;
            if (spans[spanIndex].bottom == VoxelArea.InvalidSpanValue)
            {
                voxelArea.compactCells[x + z] = new CompactVoxelCell(0, 0);
                continue;
            }

            uint index = idx;
            uint count = 0;
            while (spanIndex != -1)
            {
                if (spans[spanIndex].area != UnwalkableArea)
                {
                    int bottom = (int)spans[spanIndex].top;
                    int next = spans[spanIndex].next;
                    int top = next != -1 ? (int)spans[next].bottom : VoxelArea.MaxHeightInt;

                    if (next != -1)
                    {
                        // 优化区域时，layerflag也要合并
                        if (spans[next].area > 0)
                            spans[next].area |= spans[spanIndex].area;
                        spans[spanIndex].area = UnwalkableArea;
                    }

                    voxelArea.compactSpans[idx] = new CompactVoxelSpan((ushort)(bottom > 0xFFFF ? 0xFFFF : bottom), (uint)(top - bottom > 0xFFFF ? 0xFFFF : top - bottom));
                    voxelArea.areaTypes[idx] = spans[spanIndex].area;
                    idx++;
                    count++;
                }

                spanIndex = spans[spanIndex].next;
            }

            voxelArea.compactCells[x + z] = new CompactVoxelCell(index, count);
        }
    }
}
```



### 结构体

#### CompactVoxelSpan

```c#
/*
 * CompactVoxelSpan used for recast graphs.
 */
public struct CompactVoxelSpan
{
    public ushort y;
    public uint con;
    public uint h;
    public int reg;

    public CompactVoxelSpan(ushort bottom, uint height)
    {
        con = 24;
        y = bottom;
        h = height;
        reg = 0;
    }

    public void SetConnection(int dir, uint value)
    {
        int shift = dir * 6;
        con = (uint)((con & ~(0x3f << shift)) | ((value & 0x3f) << shift));
    }

    public int GetConnection(int dir)
    {
        return ((int)con >> dir * 6) & 0x3f;
    }
}
```

#### CompactVoxelCell

```c#
/*
 * CompactVoxelCell used for recast graphs.
 */
public struct CompactVoxelCell
{
    public uint index;
    public uint count;

    public CompactVoxelCell(uint i, uint c)
    {
        index = i;
        count = c;
    }
}
```

## 构建体素连接关系

### BuildVoxelConnections

```c#
public void BuildVoxelConnections()
{
    int wd = voxelArea.width * voxelArea.depth;

    CompactVoxelSpan[] spans = voxelArea.compactSpans;
    CompactVoxelCell[] cells = voxelArea.compactCells;

    //Build voxel connections
    for (int z = 0, pz = 0; z < wd; z += voxelArea.width, pz++)
    {
        for (int x = 0; x < voxelArea.width; x++)
        {
            CompactVoxelCell c = cells[x + z];
            for (int i = (int)c.index, ni = (int)(c.index + c.count); i < ni; i++)
            {
                CompactVoxelSpan s = spans[i];

                spans[i].con = 0xFFFFFFFF;
                for (int d = 0; d < 4; d++)
                {
                    int nx = x + voxelArea.DirectionX[d];
                    int nz = z + voxelArea.DirectionZ[d];
                    
                    if (nx < 0 || nz < 0 || nz >= wd || nx >= voxelArea.width)
                        continue;

                    CompactVoxelCell nc = cells[nx + nz];
                    for (int k = (int)nc.index, nk = (int)(nc.index + nc.count); k < nk; k++)
                    {
                        CompactVoxelSpan ns = spans[k];

                        int bottom = System.Math.Max(s.y, ns.y);
                        int top = AstarMath.Min((int)s.y + (int)s.h, (int)ns.y + (int)ns.h);
                        if ((top - bottom) >= voxelWalkableHeight && System.Math.Abs((int)ns.y - (int)s.y) <= voxelWalkableClimb)
                        {
                            uint connIdx = (uint)k - nc.index;

                            if (connIdx > MaxLayers)
                            {
                                SGameLog.Error("Too many layers");
                                continue;
                            }

                            spans[i].SetConnection(d, connIdx);
                            break;
                        }
                    }
                }
            }
        }
    }
}
```



## 去除边缘（ErodeWalkableArea）

### ErodeWalkableArea

```c#
public void ErodeWalkableArea(int radius)
{
    ushort[] src = voxelArea.tmpUShortArr;
    if (src == null || src.Length < voxelArea.compactSpanCount)
    {
        src = voxelArea.tmpUShortArr = new ushort[voxelArea.compactSpanCount];
    }

    Pathfinding.Util.Memory.MemSet<ushort>(src, 0xffff, sizeof(ushort));

    //计算每个span距离边缘的距离，用于判断某个span是否是可通行的
    CalculateDistanceField(src);

    for (int i = 0; i < src.Length; i++)
    {
        //Note multiplied with 2 because the distance field increments distance by 2 for each voxel (and 3 for diagonal)
        if (src[i] < radius * 2)
        {
            voxelArea.areaTypes[i] = UnwalkableArea;
        }
    }
}
```



### CalculateDistanceField

```c#
public ushort CalculateDistanceField(ushort[] src)
{
    int wd = voxelArea.width * voxelArea.depth;

    //Mark boundary cells
    for (int z = 0; z < wd; z += voxelArea.width)
    {
        for (int x = 0; x < voxelArea.width; x++)
        {
            CompactVoxelCell c = voxelArea.compactCells[x + z];
            for (int i = (int)c.index, ci = (int)(c.index + c.count); i < ci; i++)
            {
                CompactVoxelSpan s = voxelArea.compactSpans[i];
                int nc = 0;
                for (int d = 0; d < 4; d++)
                {
                    if (s.GetConnection(d) != NotConnected)
                    {
                        nc++;
                    }
                    else
                    {
                        break;
                    }
                }

                if (nc != 4)
                {
                    src[i] = 0;
                }
            }
        }
    }

    //Pass 1
    for (int z = 0; z < wd; z += voxelArea.width)
    {
        for (int x = 0; x < voxelArea.width; x++)
        {
            CompactVoxelCell c = voxelArea.compactCells[x + z];
            for (int i = (int)c.index, ci = (int)(c.index + c.count); i < ci; i++)
            {
                CompactVoxelSpan s = voxelArea.compactSpans[i];
                if (s.GetConnection(0) != NotConnected)
                {
                    // (-1,0)
                    int nx = x + voxelArea.DirectionX[0];
                    int nz = z + voxelArea.DirectionZ[0];

                    int ni = (int)(voxelArea.compactCells[nx + nz].index + s.GetConnection(0));

                    if (src[ni] + 2 < src[i])
                    {
                        src[i] = (ushort)(src[ni] + 2);
                    }

                    CompactVoxelSpan ns = voxelArea.compactSpans[ni];
                    if (ns.GetConnection(3) != NotConnected)
                    {
                        // (-1,0) + (0,-1) = (-1,-1)
                        int nnx = nx + voxelArea.DirectionX[3];
                        int nnz = nz + voxelArea.DirectionZ[3];

                        int nni = (int)(voxelArea.compactCells[nnx + nnz].index + ns.GetConnection(3));

                        if (src[nni] + 3 < src[i])
                        {
                            src[i] = (ushort)(src[nni] + 3);
                        }
                    }
                }

                if (s.GetConnection(3) != NotConnected)
                {
                    // (0,-1)
                    int nx = x + voxelArea.DirectionX[3];
                    int nz = z + voxelArea.DirectionZ[3];

                    int ni = (int)(voxelArea.compactCells[nx + nz].index + s.GetConnection(3));

                    if (src[ni] + 2 < src[i])
                    {
                        src[i] = (ushort)(src[ni] + 2);
                    }

                    CompactVoxelSpan ns = voxelArea.compactSpans[ni];

                    if (ns.GetConnection(2) != NotConnected)
                    {
                        // (0,-1) + (1,0) = (1,-1)
                        int nnx = nx + voxelArea.DirectionX[2];
                        int nnz = nz + voxelArea.DirectionZ[2];

                        int nni = (int)(voxelArea.compactCells[nnx + nnz].index + ns.GetConnection(2));
                        if (src[nni] + 3 < src[i])
                        {
                            src[i] = (ushort)(src[nni] + 3);
                        }
                    }
                }
            }
        }
    }

    //Pass 2
    for (int z = wd - voxelArea.width; z >= 0; z -= voxelArea.width)
    {
        for (int x = voxelArea.width - 1; x >= 0; x--)
        {
            CompactVoxelCell c = voxelArea.compactCells[x + z];
            for (int i = (int)c.index, ci = (int)(c.index + c.count); i < ci; i++)
            {
                CompactVoxelSpan s = voxelArea.compactSpans[i];
                if (s.GetConnection(2) != NotConnected)
                {
                    // (-1,0)
                    int nx = x + voxelArea.DirectionX[2];
                    int nz = z + voxelArea.DirectionZ[2];

                    int ni = (int)(voxelArea.compactCells[nx + nz].index + s.GetConnection(2));

                    if (src[ni] + 2 < src[i])
                    {
                        src[i] = (ushort)(src[ni] + 2);
                    }

                    CompactVoxelSpan ns = voxelArea.compactSpans[ni];
                    if (ns.GetConnection(1) != NotConnected)
                    {
                        // (-1,0) + (0,-1) = (-1,-1)
                        int nnx = nx + voxelArea.DirectionX[1];
                        int nnz = nz + voxelArea.DirectionZ[1];

                        int nni = (int)(voxelArea.compactCells[nnx + nnz].index + ns.GetConnection(1));

                        if (src[nni] + 3 < src[i])
                        {
                            src[i] = (ushort)(src[nni] + 3);
                        }
                    }
                }

                if (s.GetConnection(1) != NotConnected)
                {
                    // (0,-1)
                    int nx = x + voxelArea.DirectionX[1];
                    int nz = z + voxelArea.DirectionZ[1];

                    int ni = (int)(voxelArea.compactCells[nx + nz].index + s.GetConnection(1));

                    if (src[ni] + 2 < src[i])
                    {
                        src[i] = (ushort)(src[ni] + 2);
                    }

                    CompactVoxelSpan ns = voxelArea.compactSpans[ni];
                    if (ns.GetConnection(0) != NotConnected)
                    {
                        // (0,-1) + (1,0) = (1,-1)
                        int nnx = nx + voxelArea.DirectionX[0];
                        int nnz = nz + voxelArea.DirectionZ[0];

                        int nni = (int)(voxelArea.compactCells[nnx + nnz].index + ns.GetConnection(0));

                        if (src[nni] + 3 < src[i])
                        {
                            src[i] = (ushort)(src[nni] + 3);
                        }
                    }
                }
            }
        }
    }

    ushort maxDist = 0;

    for (int i = 0; i < voxelArea.compactSpanCount; i++)
    {
        maxDist = System.Math.Max(src[i], maxDist);
    }

    return maxDist;
}
```



## 计算边缘距离（BuildDistanceField）

### BuildDistanceField

```c#
public void BuildDistanceField()
{
    ushort[] src = voxelArea.tmpUShortArr;
    if (src == null || src.Length < voxelArea.compactSpanCount)
    {
        src = voxelArea.tmpUShortArr = new ushort[voxelArea.compactSpanCount];
    }

    Pathfinding.Util.Memory.MemSet<ushort>(src, 0xffff, sizeof(ushort));

    // 计算距离场
    voxelArea.maxDistance = CalculateDistanceField(src);

    ushort[] dst = voxelArea.dist;
    if (dst == null || dst.Length < voxelArea.compactSpanCount)
    {
        dst = new ushort[voxelArea.compactSpanCount];
    }

    // 模糊处理，保证距离变化是平滑的，这样可以保证在后面使用分水岭算法时不容易出现一个个小的离散区域
    dst = BoxBlur(src, dst);

    voxelArea.dist = dst;
}
```



### BoxBlur

```c#
// 模糊化处理
public ushort[] BoxBlur(ushort[] src, ushort[] dst)
{
    ushort thr = 20;

    int wd = voxelArea.width * voxelArea.depth;
    for (int z = wd - voxelArea.width; z >= 0; z -= voxelArea.width)
    {
        for (int x = voxelArea.width - 1; x >= 0; x--)
        {
            CompactVoxelCell c = voxelArea.compactCells[x + z];
            for (int i = (int)c.index, ci = (int)(c.index + c.count); i < ci; i++)
            {
                CompactVoxelSpan s = voxelArea.compactSpans[i];
                ushort cd = src[i];
                if (cd < thr)
                {
                    dst[i] = cd;
                    continue;
                }

                int total = (int)cd;
                for (int d = 0; d < 4; d++)
                {
                    if (s.GetConnection(d) != NotConnected)
                    {
                        int nx = x + voxelArea.DirectionX[d];
                        int nz = z + voxelArea.DirectionZ[d];

                        int ni = (int)(voxelArea.compactCells[nx + nz].index + s.GetConnection(d));

                        total += (int)src[ni];
                        
                        CompactVoxelSpan ns = voxelArea.compactSpans[ni];

                        int d2 = (d + 1) & 0x3;

                        if (ns.GetConnection(d2) != NotConnected)
                        {
                            int nnx = nx + voxelArea.DirectionX[d2];
                            int nnz = nz + voxelArea.DirectionZ[d2];

                            int nni = (int)(voxelArea.compactCells[nnx + nnz].index + ns.GetConnection(d2));
                            total += (int)src[nni];
                        }
                        else
                        {
                            total += cd;
                        }
                    }
                    else
                    {
                        total += cd * 2;
                    }
                }

                dst[i] = (ushort)((total + 5) / 9F);
            }
        }
    }

    return dst;
}
```



## 分水岭算法（BuildRegions）

### BuildRegions

~~~c#
public void BuildRegions()
{
    int w = voxelArea.width;
    int d = voxelArea.depth;

    int wd = w * d;

    int expandIterations = 8;

    int spanCount = voxelArea.compactSpanCount;

    List<int> stack = Pathfinding.Util.ListPool<int>.Claim(1024);
    ushort[] srcReg = new ushort[spanCount];
    ushort[] srcDist = new ushort[spanCount];
    ushort[] dstReg = new ushort[spanCount];
    ushort[] dstDist = new ushort[spanCount];

    ushort regionId = 2;
    MarkRectWithRegion(0, borderSize, 0, d, (ushort)(regionId | BorderReg), srcReg);
    regionId++;
    MarkRectWithRegion(w - borderSize, w, 0, d, (ushort)(regionId | BorderReg), srcReg);
    regionId++;
    MarkRectWithRegion(0, w, 0, borderSize, (ushort)(regionId | BorderReg), srcReg);
    regionId++;
    MarkRectWithRegion(0, w, d - borderSize, d, (ushort)(regionId | BorderReg), srcReg);
    regionId++;

    // 从最深的距离开始找，一步步减小距离，保证level是偶数
    uint level = (uint)((voxelArea.maxDistance + 1) & ~1);

    int count = 0;

    while (level > 0)
    {
        //level可以看作是分水岭算法的高度
        level = level >= 2 ? level - 2 : 0;

        //SGameLog.Debug("level:" + level.ToString());
        //PrintMap("srcReg", srcReg);
        //PrintMap("srcDist", srcDist);
        if (ExpandRegions(expandIterations, level, srcReg, srcDist, dstReg, dstDist, stack) != srcReg)
        {
            ushort[] tmp = srcReg;
            srcReg = dstReg;
            dstReg = tmp;

            tmp = srcDist;
            srcDist = dstDist;
            dstDist = tmp;
        }

        // Mark new regions with IDs.
        // 查找 “盆地”
        for (int z = 0, pz = 0; z < wd; z += w, pz++)
        {
            for (int x = 0; x < voxelArea.width; x++)
            {
                CompactVoxelCell c = voxelArea.compactCells[z + x];

                for (int i = (int)c.index, ni = (int)(c.index + c.count); i < ni; i++)
                {
                    if (voxelArea.dist[i] < level || srcReg[i] != 0 || voxelArea.areaTypes[i] == UnwalkableArea)
                        continue;

                    if (FloodRegion(x, z, i, level, regionId, srcReg, srcDist, stack))
                    {
                        //洪泛查找划分一个区域完成后区域id++标注下一个区域
                        regionId++;
                    }
                }
            }
        }

        count++;
    }

    if (ExpandRegions(expandIterations * 8, 0, srcReg, srcDist, dstReg, dstDist, stack) != srcReg)
    {
        ushort[] tmp = srcReg;
        srcReg = dstReg;
        dstReg = tmp;

        tmp = srcDist;
        srcDist = dstDist;
        dstDist = tmp;
    }

    // 过滤小于minRegionSize的区域
    voxelArea.maxRegions = regionId;

    FilterSmallRegions(srcReg, minRegionSize, voxelArea.maxRegions);

    // 保存结果
    for (int i = 0; i < voxelArea.compactSpanCount; i++)
    {
        voxelArea.compactSpans[i].reg = srcReg[i];
    }

    Pathfinding.Util.ListPool<int>.Release(stack);

    PrintMap("srcReg", srcReg);
}
~~~



### ExpandRegions

作为分水岭算法的一部分，ExpandRegions的任务是基于传入的level找到所有比level高的区域，分水岭算法中的深度在这里指代某区域到边界的最小距离

~~~c#
public ushort[] ExpandRegions(int maxIterations, uint level, ushort[] srcReg, ushort[] srcDist, ushort[] dstReg,
    ushort[] dstDist, List<int> stack)
{
    int w = voxelArea.width;
    int d = voxelArea.depth;

    int wd = w * d;

    // Find cells revealed by the raised level.
    stack.Clear();

    for (int z = 0, pz = 0; z < wd; z += w, pz++)
    {
        for (int x = 0; x < voxelArea.width; x++)
        {
            CompactVoxelCell c = voxelArea.compactCells[z + x];

            for (int i = (int)c.index, ni = (int)(c.index + c.count); i < ni; i++)
            {
                //如果某单元格到边界的最小距离大于level并且该单元格还没有被编号且可通行则将其加入到stack中
                if (voxelArea.dist[i] >= level && srcReg[i] == 0 && voxelArea.areaTypes[i] != UnwalkableArea)
                {
                    stack.Add(x);
                    stack.Add(z);
                    stack.Add(i);
                    //Debug.DrawRay (ConvertPosition(x,z,i),Vector3.up*0.5F,Color.cyan);
                }
            }
        }
    }

    int iter = 0;
    int stCount = stack.Count;

    if (stCount > 0)
    {
        while (true)
        {
            int failed = 0;

            //先将数据拷贝到dstReg和dstDist中，因为后面涉及到对srcReg和srcDist的修改
            System.Buffer.BlockCopy(srcReg, 0, dstReg, 0, srcReg.Length * sizeof(ushort));
            System.Buffer.BlockCopy(srcDist, 0, dstDist, 0, dstDist.Length * sizeof(ushort));

            for (int j = 0; j < stCount; j += 3)
            {
                if (j >= stCount)
                    break;

                int x = stack[j];
                int z = stack[j + 1];
                int i = stack[j + 2];

                if (i < 0)
                {
                    //Debug.DrawRay (ConvertPosition(x,z,i),Vector3.up*2,Color.blue);
                    failed++;
                    continue;
                }

                ushort r = srcReg[i];
                ushort d2 = 0xffff;

                CompactVoxelSpan s = voxelArea.compactSpans[i];
                int area = voxelArea.areaTypes[i];

                //检测与该单元格相邻的上下左右四个单元格
                for (int dir = 0; dir < 4; dir++)
                {
                    if (s.GetConnection(dir) == NotConnected)
                        continue;

                    int nx = x + voxelArea.DirectionX[dir];
                    int nz = z + voxelArea.DirectionZ[dir];

                    int ni = (int)voxelArea.compactCells[nx + nz].index + s.GetConnection(dir);

                    if (area != voxelArea.areaTypes[ni])
                        continue;

                    //如果相邻的单元格已经被划分而且不是边境单元格
                    //注意在BuildRegions函数中的while结构体调用ExpandRegions时一定不会进入，因为srcReg[ni]一定是0
                    //出while后的调用才会进入，因为这时候已经使用洪泛计算标注了各个area的对应region
                    if (srcReg[ni] > 0 && (srcReg[ni] & BorderReg) == 0)
                    {
                        if ((int)srcDist[ni] + 2 < (int)d2)
                        {
                            r = srcReg[ni];
                            d2 = (ushort)(srcDist[ni] + 2);
                        }
                    }
                }

                if (r != 0)
                {
                    stack[j + 2] = -1; // mark as used
                    dstReg[i] = r;
                    dstDist[i] = d2;
                }
                else
                {
                    failed++;
                    //Debug.DrawRay (ConvertPosition(x,z,i),Vector3.up*2,Color.red);
                }
            }

            // Swap source and dest.
            ushort[] tmp = srcReg;
            srcReg = dstReg;
            dstReg = tmp;

            tmp = srcDist;
            srcDist = dstDist;
            dstDist = tmp;

            //这里意味着所有在stack中的单元格的信息都已经找好了
            if (failed * 3 >= stCount)
            {
                //SGameLog.Debug("Failed count broke "+failed);
                break;
            }

            if (level > 0)
            {
                iter++;

                if (iter >= maxIterations)
                {
                    //SGameLog.Debug("Iterations broke");
                    break;
                }
            }
        }
    }

    return srcReg;
}
~~~



### FloodRegion

~~~c#
public bool FloodRegion(int x, int z, int i, uint level, ushort r, ushort[] srcReg, ushort[] srcDist, List<int> stack)
{
    int area = voxelArea.areaTypes[i];

    // Flood fill mark region.
    stack.Clear();

    stack.Add(x);
    stack.Add(z);
    stack.Add(i);

    srcReg[i] = r;
    srcDist[i] = 0;

    int lev = (int)(level >= 2 ? level - 2 : 0);

    int count = 0;

    while (stack.Count > 0)
    {
        //Similar to the Pop operation of an array, but Pop is not implemented in List<>
        int ci = stack[stack.Count - 1];
        stack.RemoveAt(stack.Count - 1);
        int cz = stack[stack.Count - 1];
        stack.RemoveAt(stack.Count - 1);
        int cx = stack[stack.Count - 1];
        stack.RemoveAt(stack.Count - 1);

        //Debug.DrawRay (ConvertPosition(cx,cz,ci),Vector3.up, Color.cyan);

        // Check if any of the neighbours already have a valid region set.
        ushort ar = 0;
        CompactVoxelSpan cs = voxelArea.compactSpans[ci];
        for (int dir = 0; dir < 4; dir++)
        {
            // 8 connected， 可以看成3 x 3的格子
            if (cs.GetConnection(dir) != NotConnected)
            {
                int ax = cx + voxelArea.DirectionX[dir];
                int az = cz + voxelArea.DirectionZ[dir];

                int ai = (int)voxelArea.compactCells[ax + az].index + cs.GetConnection(dir);

                if (voxelArea.areaTypes[ai] != area)
                    continue;

                ushort nr = srcReg[ai];
                if ((nr & BorderReg) == BorderReg) // 不考虑边界区域
                    continue;

                if (nr != 0 && nr != r)
                    ar = nr;

                CompactVoxelSpan aspan = voxelArea.compactSpans[ai];
                int dir2 = (dir + 1) & 0x3;
                if (aspan.GetConnection(dir2) != NotConnected)
                {
                    int ax2 = ax + voxelArea.DirectionX[dir2];
                    int az2 = az + voxelArea.DirectionZ[dir2];

                    int ai2 = (int)voxelArea.compactCells[ax2 + az2].index + aspan.GetConnection(dir2);

                    if (voxelArea.areaTypes[ai2] != area)
                        continue;

                    nr = srcReg[ai2];
                    if (nr != 0 && nr != r)
                    {
                        ar = nr;
                    }
                }
            }
        }

        if (ar != 0)
        {
            srcReg[ci] = 0;
            continue;
        }

        count++;

        // Expand neighbours.
        for (int dir = 0; dir < 4; ++dir)
        {
            if (cs.GetConnection(dir) != NotConnected)
            {
                int ax = cx + voxelArea.DirectionX[dir];
                int az = cz + voxelArea.DirectionZ[dir];
                int ai = (int)voxelArea.compactCells[ax + az].index + cs.GetConnection(dir);

                if (voxelArea.areaTypes[ai] != area)
                    continue;

                if (voxelArea.dist[ai] >= lev && srcReg[ai] == 0)
                {
                    srcReg[ai] = r;
                    srcDist[ai] = 0;

                    stack.Add(ax);
                    stack.Add(az);
                    stack.Add(ai);
                }
            }
        }
    }


    return count > 0;
}
~~~



### FilterSmallRegions

~~~c#
/** Filters out or merges small regions.*/
public void FilterSmallRegions(ushort[] reg, int minRegionSize, int maxRegions)
{
    RelevantGraphSurface c = RelevantGraphSurface.Root;
    bool anySurfaces = c != null && (relevantGraphSurfaceMode != RelevantGraphSurfaceMode.DoNotRequire);

    // Nothing to do here
    if (!anySurfaces && minRegionSize <= 0)
        return;

    int[] counter = new int[maxRegions];
    ushort[] bits = voxelArea.tmpUShortArr;
    if (bits == null || bits.Length < maxRegions)
    {
        bits = voxelArea.tmpUShortArr = new ushort[maxRegions];
    }

    Util.Memory.MemSet(counter, -1, sizeof(int));
    Util.Memory.MemSet(bits, (ushort)0, maxRegions, sizeof(ushort));

    int nReg = counter.Length;
    int wd = voxelArea.width * voxelArea.depth;

    const int RelevantSurfaceSet = 1 << 1;
    const int BorderBit = 1 << 0;

    // Mark RelevantGraphSurfaces

    // If they can also be adjacent to tile borders, this will also include the BorderBit
    int RelevantSurfaceCheck = RelevantSurfaceSet |
                               ((relevantGraphSurfaceMode == RelevantGraphSurfaceMode.OnlyForCompletelyInsideTile)
                                   ? BorderBit
                                   : 0x0);

    if (anySurfaces)
    {
        while (c != null)
        {
            int x, z;
            VectorToIndex(c.Position, out x, out z);

            // Out of bounds
            if (x < 0 || z < 0 || x >= voxelArea.width || z >= voxelArea.depth)
            {
                c = c.Next;
                continue;
            }

            int y = (int)((c.Position.y - voxelOffset.y) / cellHeight);
            int rad = (int)(c.maxRange / cellHeight);

            CompactVoxelCell cell = voxelArea.compactCells[x + z * voxelArea.width];
            for (int i = (int)cell.index; i < cell.index + cell.count; i++)
            {
                CompactVoxelSpan s = voxelArea.compactSpans[i];
                if (System.Math.Abs(s.y - y) <= rad && reg[i] != 0)
                {
                    bits[union_find_find(counter, (int)reg[i] & ~BorderReg)] |= RelevantSurfaceSet;
                }
            }

            c = c.Next;
        }
    }

    for (int z = 0, pz = 0; z < wd; z += voxelArea.width, pz++)
    {
        for (int x = 0; x < voxelArea.width; x++)
        {
            CompactVoxelCell cell = voxelArea.compactCells[x + z];
            for (int i = (int)cell.index; i < cell.index + cell.count; i++)
            {
                CompactVoxelSpan s = voxelArea.compactSpans[i];
                int r = (int)reg[i];
                if ((r & ~BorderReg) == 0)
                    continue;

                if (r >= nReg)
                {
                    //Probably border
                    bits[union_find_find(counter, r & ~BorderReg)] |= BorderBit;
                    continue;
                }

                int k = union_find_find(counter, r);

                // Count this span
                counter[k]--;

                for (int dir = 0; dir < 4; dir++)
                {
                    if (s.GetConnection(dir) == NotConnected)
                        continue;

                    int nx = x + voxelArea.DirectionX[dir];
                    int nz = z + voxelArea.DirectionZ[dir];

                    int ni = (int)voxelArea.compactCells[nx + nz].index + s.GetConnection(dir);

                    int r2 = (int)reg[ni];

                    if (r != r2 && (r2 & ~BorderReg) != 0)
                    {
                        if ((r2 & BorderReg) != 0)
                        {
                            bits[k] |= BorderBit;
                        }
                        else
                        {
                            union_find_union(counter, k, r2);
                        }
                    }
                }
            }
        }
    }

    // Propagate bits
    for (int i = 0; i < counter.Length; i++) bits[union_find_find(counter, i)] |= bits[i];

    for (int i = 0; i < counter.Length; i++)
    {
        int ctr = union_find_find(counter, i);

        // Adjacent to border
        if ((bits[ctr] & BorderBit) != 0)
        {
            counter[ctr] = -minRegionSize - 2;
        }

        // Not in any relevant surface
        // or it is adjacent to a border (see RelevantSurfaceCheck)
        if (anySurfaces && (bits[ctr] & RelevantSurfaceCheck) == 0)
        {
            counter[ctr] = -1;
        }
    }

    for (int i = 0; i < voxelArea.compactSpanCount; i++)
    {
        int r = (int)reg[i];
        if (r >= nReg)
            continue;

        if (counter[union_find_find(counter, r)] >= -minRegionSize - 1)
        {
            reg[i] = 0;
        }
    }
}
~~~



## 获取边界（BuildContours）

​	这一步根据上一步划定的区域找到各个区域的边界

### BuildContours

~~~c#
public void BuildContours(float maxError, int maxEdgeLength, VoxelContourSet cset, int buildFlags)
{
    int w = voxelArea.width;
    int d = voxelArea.depth;

    int wd = w * d;

    int maxContours = Mathf.Max(8, 8);
    List<VoxelContour> contours = new List<VoxelContour>(maxContours);

    //NOTE: This array may contain any data, but since we explicitly set all data in it before we use it, it's OK.
    ushort[] flags = voxelArea.tmpUShortArr;
    if (flags.Length < voxelArea.compactSpanCount)
    {
        flags = voxelArea.tmpUShortArr = new ushort[voxelArea.compactSpanCount];
    }

    // 标记边界
    for (int z = 0; z < wd; z += voxelArea.width)
    {
        for (int x = 0; x < voxelArea.width; x++)
        {
            CompactVoxelCell c = voxelArea.compactCells[x + z];

            for (int i = (int)c.index, ci = (int)(c.index + c.count); i < ci; i++)
            {
                ushort res = 0;
                CompactVoxelSpan s = voxelArea.compactSpans[i];

                if (s.reg == 0 || (s.reg & BorderReg) == BorderReg)
                {
                    flags[i] = 0;
                    continue;
                }

                for (int dir = 0; dir < 4; dir++)
                {
                    int r = 0;
                    if (s.GetConnection(dir) != NotConnected)
                    {
                        int nx = x + voxelArea.DirectionX[dir];
                        int nz = z + voxelArea.DirectionZ[dir];

                        int ni = (int)voxelArea.compactCells[nx + nz].index + s.GetConnection(dir);
                        r = voxelArea.compactSpans[ni].reg;
                    }

                    //@TODO - Why isn't this inside the previous IF
                    if (r == s.reg)
                    {
                        res |= (ushort)(1 << dir);
                    }
                }

                //取反，标记未连接的边线（二进制四位每位表示一个方向是否是与同区域的块相连的，1表示当前区域与某方向的临近区域region id不同）
                flags[i] = (ushort)(res ^ 0xf);
            }
        }
    }

    List<int> verts = Pathfinding.Util.ListPool<int>.Claim(256); //new List<int> (256);
    List<int> simplified = Pathfinding.Util.ListPool<int>.Claim(64); //new List<int> (64);

    for (int z = 0; z < wd; z += voxelArea.width)
    {
        for (int x = 0; x < voxelArea.width; x++)
        {
            CompactVoxelCell c = voxelArea.compactCells[x + z];

            for (int i = (int)c.index, ci = (int)(c.index + c.count); i < ci; i++)
            {
                // 在上一个区域遍历的过程中，将四方向与自己区域ID都相同的区域flag设为0xf，将四个方向都不同的区域flag设为0，此时就continue
                if (flags[i] == 0 || flags[i] == 0xf)
                {
                    flags[i] = 0;
                    continue;
                }

               	// 排除掉空白区域和border
                int reg = voxelArea.compactSpans[i].reg;
                if (reg == 0 || (reg & BorderReg) == BorderReg)
                    continue;

                // 那么剩下的就是区域边界的点了
                int area = voxelArea.areaTypes[i];

                verts.Clear();
                simplified.Clear();

                WalkContour(x, z, i, flags, verts); // 从一个边界点起始，顺时针滚动一圈，刚好找完所有连续的点verts

                SimplifyContour(verts, simplified, maxError, maxEdgeLength, buildFlags); // verts简化到simplified
                RemoveDegenerateSegments(simplified);

                VoxelContour contour = new VoxelContour();
                contour.verts = ClaimIntArr(simplified.Count, false); //simplified.ToArray ();
                for (int j = 0; j < simplified.Count; j++)
                {
                    contour.verts[j] = simplified[j];
                }
                
                contour.nverts = simplified.Count / 4;
                contour.reg = reg;
                contour.area = area;

                contours.Add(contour);
            }
        }
    }

    Pathfinding.Util.ListPool<int>.Release(verts);
    Pathfinding.Util.ListPool<int>.Release(simplified);

    // Check and merge droppings.
    // Sometimes the previous algorithms can fail and create several contours
    // per area. This pass will try to merge the holes into the main region.
    for (int i = 0; i < contours.Count; i++)
    {
        VoxelContour cont = contours[i];
        // Check if the contour is would backwards.
        if (CalcAreaOfPolygon2D(cont.verts, cont.nverts) < 0)
        {
            // Find another contour which has the same region ID.
            int mergeIdx = -1;
            for (int j = 0; j < contours.Count; j++)
            {
                if (i == j) continue;
                if (contours[j].nverts > 0 && contours[j].reg == cont.reg)
                {
                    // Make sure the polygon is correctly oriented.
                    if (CalcAreaOfPolygon2D(contours[j].verts, contours[j].nverts) > 0)
                    {
                        mergeIdx = j;
                        break;
                    }
                }
            }

            if (mergeIdx == -1)
            {
                SGameLog.Error("rcBuildContours: Could not find merge target for bad contour " + i + ".");
            }
            else
            {
                SGameLog.Warning("Fixing contour");
                VoxelContour mcont = contours[mergeIdx];

                // Merge by closest points.
                int ia = 0, ib = 0;
                GetClosestIndices(mcont.verts, mcont.nverts, cont.verts, cont.nverts, ref ia, ref ib);

                if (ia == -1 || ib == -1)
                {
                    SGameLog.Warning("rcBuildContours: Failed to find merge points for " + i + " and " + mergeIdx +
                                     ".");
                    continue;
                }

                if (!MergeContours(ref mcont, ref cont, ia, ib))
                {
                    SGameLog.Warning("rcBuildContours: Failed to merge contours " + i + " and " + mergeIdx + ".");
                    continue;
                }

                contours[mergeIdx] = mcont;
                contours[i] = cont;
            }
        }
    }

    cset.conts = contours;
}
~~~



### WalkContour

~~~c#
public void WalkContour(int x, int z, int i, ushort[] flags, List<int> verts)
{
    /*
    *    DirectionX = new int[4] { -1,     0, 1,      0 };
    *    DirectionZ = new int[4] {  0, width, 0, -width };
    */
    // Choose the first non-connected edge
    int dir = 0;

    
    while ((flags[i] & (ushort)(1 << dir)) == 0)
    {
        dir++;
    }

    int startDir = dir;
    int startI = i;

    int area = voxelArea.areaTypes[i];
    int iter = 0;

    while (iter++ < 40000)
    {
        //站在当前区域看向dir方向是不是另一个region id的区域
        if ((flags[i] & (ushort)(1 << dir)) != 0)
        {
            //Choose the edge corner
            bool isBorderVertex = false;
            bool isAreaBorder = false;

            int px = x;
            int py = GetCornerHeight(x, z, i, dir, ref isBorderVertex);
            int pz = z;

            /*
           	*    case 1: px++; break;
            *    case 2: px++; pz += voxelArea.width; break;
            *    case 3: pz += voxelArea.width; break;
            */
            switch (dir)
            {
                case 0:
                    pz += voxelArea.width;
                    break;
                case 1:
                    px++;
                    pz += voxelArea.width;
                    break;
                case 2:
                    px++;
                    break;
            }

            int r = 0;
            CompactVoxelSpan s = voxelArea.compactSpans[i];

            if (s.GetConnection(dir) != NotConnected)
            {
                int nx = x + voxelArea.DirectionX[dir];
                int nz = z + voxelArea.DirectionZ[dir];
                int ni = (int)voxelArea.compactCells[nx + nz].index + s.GetConnection(dir);
                r = (int)voxelArea.compactSpans[ni].reg;

                //如果连区域类型都不一样说明走到整个大区域的边界了（多个region构成的大区域）
                if (area != voxelArea.areaTypes[ni])
                {
                    isAreaBorder = true;
                }
            }

            //在二进制中添加两个信息，节省存储空间
            if (isBorderVertex)
            {
                r |= RC_BORDER_VERTEX; //65536 0001 0000 0000 0000 0000
            }

            if (isAreaBorder)
            {
                r |= RC_AREA_BORDER;  //131072 0010 0000 0000 0000 0000
            }

            verts.Add(px);
            verts.Add(py);
            verts.Add(pz);
            verts.Add(r);

            //Debug.DrawRay (previousPos,new Vector3 ((dir == 1 || dir == 2) ? 1 : 0, 0, (dir == 0 || dir == 1) ? 1 : 0),Color.cyan);

            //找到后就将这条路线移除防止无限循环
            flags[i] = (ushort)(flags[i] & ~(1 << dir)); // Remove visited edges

            dir = (dir + 1) & 0x3; // 顺时针旋转
        }
        else
        {
            //站在当前区域看向dir方向不是另一个region id的区域就往该方向走一格，更新x,z,i
            int ni = -1;
            int nx = x + voxelArea.DirectionX[dir];
            int nz = z + voxelArea.DirectionZ[dir];

            CompactVoxelSpan s = voxelArea.compactSpans[i];

            if (s.GetConnection(dir) != NotConnected)
            {
                CompactVoxelCell nc = voxelArea.compactCells[nx + nz];
                ni = (int)nc.index + s.GetConnection(dir);
            }

            if (ni == -1)
            {
                SGameLog.Error("This should not happen");
                return;
            }

            x = nx;
            z = nz;
            i = ni;

            dir = (dir + 3) & 0x3; // 逆时针旋转
        }

        //绕一圈回来说明已经找到一圈边界了，返回
        if (startI == i && startDir == dir)
        {
            break;
        }
    }
}
~~~



### GetCornerHeight

~~~c#
public int GetCornerHeight(int x, int z, int i, int dir, ref bool isBorderVertex)
{
    CompactVoxelSpan s = voxelArea.compactSpans[i];

    int ch = (int)s.y;

    //顺时针旋转
    int dirp = (dir + 1) & 0x3;

    uint[] regs = new uint[4];

    regs[0] = (uint)voxelArea.compactSpans[i].reg | ((uint)voxelArea.areaTypes[i] << 16);

    if (s.GetConnection(dir) != NotConnected)
    {
        int nx = x + voxelArea.DirectionX[dir];
        int nz = z + voxelArea.DirectionZ[dir];
        int ni = (int)voxelArea.compactCells[nx + nz].index + s.GetConnection(dir);

        CompactVoxelSpan ns = voxelArea.compactSpans[ni];

        ch = AstarMath.Max(ch, (int)ns.y);
        regs[1] = (uint)ns.reg | ((uint)voxelArea.areaTypes[ni] << 16);

        if (ns.GetConnection(dirp) != NotConnected)
        {
            int nx2 = nx + voxelArea.DirectionX[dirp];
            int nz2 = nz + voxelArea.DirectionZ[dirp];
            int ni2 = (int)voxelArea.compactCells[nx2 + nz2].index + ns.GetConnection(dirp);

            CompactVoxelSpan ns2 = voxelArea.compactSpans[ni2];

            ch = AstarMath.Max(ch, (int)ns2.y);
            regs[2] = (uint)ns2.reg | ((uint)voxelArea.areaTypes[ni2] << 16);
        }
    }

    if (s.GetConnection(dirp) != NotConnected)
    {
        int nx = x + voxelArea.DirectionX[dirp];
        int nz = z + voxelArea.DirectionZ[dirp];
        int ni = (int)voxelArea.compactCells[nx + nz].index + s.GetConnection(dirp);

        CompactVoxelSpan ns = voxelArea.compactSpans[ni];

        ch = AstarMath.Max(ch, (int)ns.y);
        regs[3] = (uint)ns.reg | ((uint)voxelArea.areaTypes[ni] << 16);

        if (ns.GetConnection(dir) != NotConnected)
        {
            int nx2 = nx + voxelArea.DirectionX[dir];
            int nz2 = nz + voxelArea.DirectionZ[dir];
            int ni2 = (int)voxelArea.compactCells[nx2 + nz2].index + ns.GetConnection(dir);

            CompactVoxelSpan ns2 = voxelArea.compactSpans[ni2];

            ch = AstarMath.Max(ch, (int)ns2.y);
            regs[2] = (uint)ns2.reg | ((uint)voxelArea.areaTypes[ni2] << 16);
        }
    }

    // Check if the vertex is special edge vertex, these vertices will be removed later.
    for (int j = 0; j < 4; ++j)
    {
        int a = j;
        int b = (j + 1) & 0x3;
        int c = (j + 2) & 0x3;
        int d = (j + 3) & 0x3;

        // The vertex is a border vertex there are two same exterior cells in a row,
        // followed by two interior cells and none of the regions are out of bounds.
        bool twoSameExts = (regs[a] & regs[b] & BorderReg) != 0 && regs[a] == regs[b];
        bool twoInts = ((regs[c] | regs[d]) & BorderReg) == 0;
        bool intsSameArea = (regs[c] >> 16) == (regs[d] >> 16);
        bool noZeros = regs[a] != 0 && regs[b] != 0 && regs[c] != 0 && regs[d] != 0;
        if (twoSameExts && twoInts && intsSameArea && noZeros)
        {
            isBorderVertex = true;
            break;
        }
    }

    return ch;
}
~~~



### SimplifyContour

​	这个函数的作用是把WalkContour中找到的破碎的轮廓线段整合简化，举例来说WalkContour的算法会把一条线分为无数个小的线段，而这个函数要做的就是把这些小的线段规整为一条线

~~~c#
public void SimplifyContour(List<int> verts, List<int> simplified, float maxError, int maxEdgeLength, int buildFlags)
{
    // Add initial points.
    bool hasConnections = false;
    for (int i = 0; i < verts.Count; i += 4)
    {
        if ((verts[i + 3] & ContourRegMask) != 0)
        {
            hasConnections = true;
            break;
        }
    }

    if (hasConnections)
    {
        // The contour has some portals to other regions.
        // Add a new point to every location where the region changes.
        // 在两个区域之间增加一个通道
        for (int i = 0, ni = verts.Count / 4; i < ni; i++)
        {
            int ii = (i + 1) % ni;
            bool differentRegs = (verts[i * 4 + 3] & ContourRegMask) != (verts[ii * 4 + 3] & ContourRegMask); //ContourRegMask 0xffff
            bool areaBorders = (verts[i * 4 + 3] & RC_AREA_BORDER) != (verts[ii * 4 + 3] & RC_AREA_BORDER);   //RC_AREA_BORDER 131072

            if (differentRegs || areaBorders)
            {
                simplified.Add(verts[i * 4 + 0]);
                simplified.Add(verts[i * 4 + 1]);
                simplified.Add(verts[i * 4 + 2]);
                simplified.Add(i);
            }
        }
    }


    if (simplified.Count == 0)
    {
        // If there is no connections at all,
        // create some initial points for the simplification process. 
        // Find lower-left and upper-right vertices of the contour.
        int llx = verts[0];
        int lly = verts[1];
        int llz = verts[2];
        int lli = 0;
        int urx = verts[0];
        int ury = verts[1];
        int urz = verts[2];
        int uri = 0;

        for (int i = 0; i < verts.Count; i += 4)
        {
            int x = verts[i + 0];
            int y = verts[i + 1];
            int z = verts[i + 2];
            if (x < llx || (x == llx && z < llz))
            {
                llx = x;
                lly = y;
                llz = z;
                lli = i / 4;
            }

            if (x > urx || (x == urx && z > urz))
            {
                urx = x;
                ury = y;
                urz = z;
                uri = i / 4;
            }
        }

        simplified.Add(llx);
        simplified.Add(lly);
        simplified.Add(llz);
        simplified.Add(lli);

        simplified.Add(urx);
        simplified.Add(ury);
        simplified.Add(urz);
        simplified.Add(uri);
    }

    // Add points until all raw points are within
    // error tolerance to the simplified shape.
    int pn = verts.Count / 4;

    //Use the max squared error instead
    maxError *= maxError;

    for (int i = 0; i < simplified.Count / 4;)
    {
        int ii = (i + 1) % (simplified.Count / 4);

        int ax = simplified[i * 4 + 0];
        int az = simplified[i * 4 + 2];
        int ai = simplified[i * 4 + 3];

        int bx = simplified[ii * 4 + 0];
        int bz = simplified[ii * 4 + 2];
        int bi = simplified[ii * 4 + 3];

        // Find maximum deviation from the segment.
        float maxd = 0;
        int maxi = -1;
        int ci, cinc, endi;

        // Traverse the segment in lexilogical order so that the
        // max deviation is calculated similarly when traversing
        // opposite segments.
        if (bx > ax || (bx == ax && bz > az))
        {
            cinc = 1;
            ci = (ai + cinc) % pn;
            endi = bi;
        }
        else
        {
            cinc = pn - 1;
            ci = (bi + cinc) % pn;
            endi = ai;
        }

        // 只细分连接轮廓和边界轮廓
        // 连接轮廓：连接两个有效区域之间的边界边。
        // 边界轮廓：有效区域与空区域之间的相交的边界边。
        if ((verts[ci * 4 + 3] & ContourRegMask) == 0 ||
            (verts[ci * 4 + 3] & RC_AREA_BORDER) == RC_AREA_BORDER)
        {
            while (ci != endi)
            {
                float d2 = AstarMath.DistancePointSegment(verts[ci * 4 + 0], verts[ci * 4 + 2] / voxelArea.width, ax,
                                                          az / voxelArea.width, bx, bz / voxelArea.width);

                if (d2 > maxd)
                {
                    maxd = d2;
                    maxi = ci;
                }

                ci = (ci + cinc) % pn;
            }
        }

        // 如果某点的最大的偏移量大于可以接受的范围就将对应的点添加到简化轮廓的集合中去，并更新
        if (maxi != -1 && maxd > maxError)
        {
            // Add space for the new point.
            //simplified.resize(simplified.size()+4);
            simplified.Add(0);
            simplified.Add(0);
            simplified.Add(0);
            simplified.Add(0);

            int n = simplified.Count / 4;

            for (int j = n - 1; j > i; --j)
            {
                simplified[j * 4 + 0] = simplified[(j - 1) * 4 + 0];
                simplified[j * 4 + 1] = simplified[(j - 1) * 4 + 1];
                simplified[j * 4 + 2] = simplified[(j - 1) * 4 + 2];
                simplified[j * 4 + 3] = simplified[(j - 1) * 4 + 3];
            }

            // Add the point.
            simplified[(i + 1) * 4 + 0] = verts[maxi * 4 + 0];
            simplified[(i + 1) * 4 + 1] = verts[maxi * 4 + 1];
            simplified[(i + 1) * 4 + 2] = verts[maxi * 4 + 2];
            simplified[(i + 1) * 4 + 3] = maxi;
        }
        else
        {
            i++;
        }
    }

    //分割太长的边
    float maxEdgeLen = maxEdgeLength / cellSize;

    if (maxEdgeLen > 0 && (buildFlags & (RC_CONTOUR_TESS_WALL_EDGES | RC_CONTOUR_TESS_AREA_EDGES)) != 0)
    {
        for (int i = 0; i < simplified.Count / 4;)
        {
            if (simplified.Count / 4 > 200)
                break;

            int ii = (i + 1) % (simplified.Count / 4);

            int ax = simplified[i * 4 + 0];
            int az = simplified[i * 4 + 2];
            int ai = simplified[i * 4 + 3];

            int bx = simplified[ii * 4 + 0];
            int bz = simplified[ii * 4 + 2];
            int bi = simplified[ii * 4 + 3];

            // Find maximum deviation from the segment.
            int maxi = -1;
            int ci = (ai + 1) % pn;

            // Tessellate only outer edges or edges between areas.
            bool tess = false;

            // Wall edges.
            if ((buildFlags & RC_CONTOUR_TESS_WALL_EDGES) == 1 && (verts[ci * 4 + 3] & ContourRegMask) == 0)
            {
                tess = true;
            }

            // Edges between areas.
            if ((buildFlags & RC_CONTOUR_TESS_AREA_EDGES) == 1 && (verts[ci * 4 + 3] & RC_AREA_BORDER) == 1)
            {
                tess = true;
            }

            if (tess)
            {
                int dx = bx - ax;
                int dz = (bz / voxelArea.width) - (az / voxelArea.width);
                if (dx * dx + dz * dz > maxEdgeLen * maxEdgeLen)
                {
                    // Round based on the segments in lexilogical order so that the
                    // max tesselation is consistent regardles in which direction
                    // segments are traversed.
                    if (bx > ax || (bx == ax && bz > az))
                    {
                        int n = bi < ai ? (bi + pn - ai) : (bi - ai);
                        maxi = (ai + n / 2) % pn;
                    }
                    else
                    {
                        int n = bi < ai ? (bi + pn - ai) : (bi - ai);
                        maxi = (ai + (n + 1) / 2) % pn;
                    }
                }
            }

            // If the max deviation is larger than accepted error,
            // add new point, else continue to next segment.
            if (maxi != -1)
            {
                // Add space for the new point.
                //simplified.resize(simplified.size()+4);
                simplified.AddRange(new int[4]);

                int n = simplified.Count / 4;
                for (int j = n - 1; j > i; --j)
                {
                    simplified[j * 4 + 0] = simplified[(j - 1) * 4 + 0];
                    simplified[j * 4 + 1] = simplified[(j - 1) * 4 + 1];
                    simplified[j * 4 + 2] = simplified[(j - 1) * 4 + 2];
                    simplified[j * 4 + 3] = simplified[(j - 1) * 4 + 3];
                }

                // Add the point.
                simplified[(i + 1) * 4 + 0] = verts[maxi * 4 + 0];
                simplified[(i + 1) * 4 + 1] = verts[maxi * 4 + 1];
                simplified[(i + 1) * 4 + 2] = verts[maxi * 4 + 2];
                simplified[(i + 1) * 4 + 3] = maxi;
            }
            else
            {
                ++i;
            }
        }
    }

    for (int i = 0; i < simplified.Count / 4; i++)
    {
        // The edge vertex flag is take from the current raw point,
        // and the neighbour region is take from the next raw point.
        int ai = (simplified[i * 4 + 3] + 1) % pn;
        int bi = simplified[i * 4 + 3];
        simplified[i * 4 + 3] = (verts[ai * 4 + 3] & ContourRegMask) | (verts[bi * 4 + 3] & RC_BORDER_VERTEX);
    }
}
~~~



### RemoveDegenerateSegments

~~~c#
public void RemoveDegenerateSegments(List<int> simplified)
{
    // Remove adjacent vertices which are equal on xz-plane,
    // or else the triangulator will get confused
    for (int i = 0; i < simplified.Count / 4; i++)
    {
        int ni = i + 1;
        if (ni >= (simplified.Count / 4))
        {
            ni = 0;
        }

        if (simplified[i * 4 + 0] == simplified[ni * 4 + 0] &&
            simplified[i * 4 + 2] == simplified[ni * 4 + 2])
        {
            // 退化的部分，移除
            simplified.RemoveRange(i, 4);
        }
    }
}
~~~



### 结构体

#### VoxelContour

~~~c#
public struct VoxelContour
{
    public int nverts; // number of vertices
    public int[] verts; // Vertex coordinates, each vertex contains 4 components.
    public int[] rverts; // Raw vertex coordinates, each vertex contains 4 components.

    public int reg; // Region ID of the contour.
    public int area; // Area ID of the contour.
}
~~~

#### VoxelContourSet

~~~c#
public class VoxelContourSet
{
    public List<VoxelContour> conts; // Pointer to all contours.
    public Bounds bounds; // 高度场的包围盒
}
~~~



## 构建mesh（BuildPolyMesh）

### BuildPolyMesh

~~~c#
/* 
 * Builds a polygon mesh from a contour set.
 * param nvp Maximum allowed vertices per polygon. (nvp: number of vertices of polygon)
 * note Currently locked to 3 
 */
public bool BuildPolyMesh(VoxelContourSet cset, int nvp, out VoxelMesh mesh)
{
    bool result = true;
  
    nvp = 3;

    int maxVertices = 0;
    int maxTris = 0;
    int maxVertsPerCont = 0;

    for (int i = 0; i < cset.conts.Count; i++)
    {
        // Skip null contours.
        if (cset.conts[i].nverts < 3) 
            continue;

        maxVertices += cset.conts[i].nverts;
        maxTris += cset.conts[i].nverts - 2;  // 三角形数量 = 顶点数-2，比如4边形最多2个三角形，5边形最多3个三角形
        maxVertsPerCont = AstarMath.Max(maxVertsPerCont, cset.conts[i].nverts);
    }

    if (maxVertices >= 65534)
    {
        SGameLog.Warning("To many vertices for unity to render - Unity might screw up rendering, but hopefully the navmesh will work ok");
    }

    /** \todo Could be cached to avoid allocations */
    VInt3[] verts = new VInt3[maxVertices];
    /** \todo Could be cached to avoid allocations */
    int[] polys = new int[maxTris * nvp];

    Pathfinding.Util.Memory.MemSet<int>(polys, 0xff, sizeof(int));

    int[] indices = new int[maxVertsPerCont];
    int[] tris = new int[maxVertsPerCont * 3]; // 一个点可能有很多三角形共用所以乘3

    int vertexIndex = 0;
    int polyIndex = 0;

    List<uint> trisArea = new List<uint>();
    for (int i = 0; i < cset.conts.Count; i++)
    {
        VoxelContour cont = cset.conts[i];

        // 跳过无意义的边界
        if (cont.nverts < 3)
            continue;

        for (int j = 0; j < cont.nverts; j++)
        {
            indices[j] = j;
            cont.verts[j * 4 + 2] /= voxelArea.width;  // 相当于求得在第几行，这样就类似于坐标点（x，z），计算它对应索引位置时要x + z * width
        }

        bool triangleResult = false;
        int ntris = Triangulate(cont.nverts, cont.verts, ref indices, ref tris, ref triangleResult);
        result = result && triangleResult;

        // tris里的数据索引是从0开始的，但是polys里保存了所有contour的三角形信息，所以添加到polys时要添加一个startIndex索引代表这个contour之前已经添加了多少其他contour的三角形
        int startIndex = vertexIndex;
        for (int j = 0; j < ntris * 3; polyIndex++, j++)
        {
            //@Error sometimes
            polys[polyIndex] = tris[j] + startIndex;  // poly里存放的是组成三角形的某个顶点在verts中的索引位置
        }

        //LayerFlag与每个三角形对应
        for (int j = 0; j < ntris; j++)
        {
            trisArea.Add((uint)cont.area);
        }
        for (int j = 0; j < cont.nverts; vertexIndex++, j++)
        {
            // 按顺序存放顶点信息保证其和polys里的索引信息是对应的
            verts[vertexIndex] = new VInt3(cont.verts[j * 4], cont.verts[j * 4 + 1], cont.verts[j * 4 + 2]);  
        }
    }

    mesh = new VoxelMesh();
    VInt3[] trimmedVerts = new VInt3[vertexIndex];
    
    // 需要放到trim里是因为之前的verts和polys都是设置的最大值，实际上可能用不到这么大的空间，会有很多空的数据
    for (int i = 0; i < vertexIndex; i++)
    {
        trimmedVerts[i] = verts[i];
    }

    int[] trimmedTris = new int[polyIndex];

    System.Buffer.BlockCopy(polys, 0, trimmedTris, 0, polyIndex * sizeof(int));

    mesh.verts = trimmedVerts;
    mesh.tris = trimmedTris;
    mesh.layerFlag = trisArea.ToArray();

    return result;
}
~~~



### Triangulate

~~~c#
public int Triangulate(int n, int[] verts, ref int[] indices, ref int[] tris, ref bool success)
{
    success = true;

    int ntris = 0;
    int[] dst = tris;

    int dstIndex = 0;

    int on = n;  //original number

    // 0x40000000是用来判断某顶点是否是潜在的可以从三角剖分集合中移去的顶点
    for (int i = 0; i < n; i++)
    {
        int i1 = Next(i, n);
        int i2 = Next(i1, n);
        // 如果i和i2是对角线则将i1添加一个可以移除的信息
        if (Diagonal(i, i2, n, verts, indices))
        {
            indices[i1] |= 0x40000000;
        }
    }

    while (n > 3)
    {
        int minLen = -1;
        int mini = -1;

        //选择长度最短的一条对角线
        for (int q = 0; q < n; q++)
        {
            int q1 = Next(q, n);
            if ((indices[q1] & 0x40000000) != 0)
            {
                //0x0fffffff用来去除添加的可移除信息，将原有数据还原以免造成数据污染
                int p0 = (indices[q] & 0x0fffffff) * 4;
                int p2 = (indices[Next(q1, n)] & 0x0fffffff) * 4;

                int dx = verts[p2 + 0] - verts[p0 + 0];
                int dz = verts[p2 + 2] - verts[p0 + 2];

                //Squared distance
                int len = dx * dx + dz * dz;

                //找到更小的更新
                if (minLen < 0 || len < minLen)
                {
                    minLen = len;
                    mini = q;
                }
            }
        }

        if (mini == -1)
        {
            SGameLog.Error("This should not happen");
            for (int j = 0; j < on; j++)
            {
                DrawLine(Prev(j, on), j, indices, verts, Color.red);
            }

            success = false;

            return -ntris;
        }

        
        // 找到最短对角线就意味着分出了一个三角形，此时将该三角形信息存入dst中，dst中每三个数据代表一个三角形的三个顶点索引信息
        int i = mini;
        int i1 = Next(i, n);
        int i2 = Next(i1, n);

        dst[dstIndex] = indices[i] & 0x0fffffff;
        dstIndex++;
        dst[dstIndex] = indices[i1] & 0x0fffffff;
        dstIndex++;
        dst[dstIndex] = indices[i2] & 0x0fffffff;
        dstIndex++;
        ntris++; // 三角形数量+1

        // 找到三角形后将i1从待划分三角形集合中移除出去，后面的全部往前移动一位
        n--;
        for (int k = i1; k < n; k++)
        {
            indices[k] = indices[k + 1];
        }

        if (i1 >= n)
        {
            i1 = 0;
        }
        i = Prev(i1, n);

        // 移除i1后判断是否有新的对角线生成，有的话添加一个可移除的信息
        if (Diagonal(Prev(i, n), i1, n, verts, indices))
        {
            //DrawLine (Prev(i,n),i1,indices,verts,Color.green);
            indices[i] |= 0x40000000;
        }
        //如果不是对角线则移除原有的可移除信息
        else
        {
            //DrawLine (Prev(i,n),i1,indices,verts,Color.white);
            indices[i] &= 0x0fffffff;
        }

        //与上面同理
        if (Diagonal(i, Next(i1, n), n, verts, indices))
        {
            //DrawLine (Next(i1, n),i,indices,verts,Color.green);
            indices[i1] |= 0x40000000;
        }
        else
        {
            indices[i1] &= 0x0fffffff;
        }
    }

    //把最后剩下的一个三角形信息存入，函数结束
    dst[dstIndex] = indices[0] & 0x0fffffff;
    dstIndex++;
    dst[dstIndex] = indices[1] & 0x0fffffff;
    dstIndex++;
    dst[dstIndex] = indices[2] & 0x0fffffff;
    dstIndex++;
    ntris++;

    return ntris;
}
~~~



### 结构体

#### VoxelSpan

~~~c#
public class VoxelSpan
{
    public uint bottom;
    public uint top;

    public VoxelSpan next;

    /*Area
     *   0 is an unwalkable span (triangle face down)
     *   1 is a walkable span (triangle face up)
     */
    public int area;

    public VoxelSpan(uint b, uint t, int area)
    {
        bottom = b;
        top = t;
        this.area = area;
    }
}
~~~

#### VoxelCell

~~~c#
/* 
 * VoxelCell used for recast graphs.
 */
public struct VoxelCell
{
    public VoxelSpan firstSpan;

    public void AddSpan(uint bottom, uint top, int area, int voxelWalkableClimb)
    {
        VoxelSpan span = new VoxelSpan(bottom, top, area);

        if (firstSpan == null)
        {
            firstSpan = span;
            return;
        }

        VoxelSpan prev = null;
        VoxelSpan cSpan = firstSpan;
        while (cSpan != null)
        {
            if (cSpan.bottom > span.top)
                break;

            if (cSpan.top < span.bottom)
            {
                prev = cSpan;
                cSpan = cSpan.next;
            }
            else
            {
                if (cSpan.bottom < bottom)
                {
                    span.bottom = cSpan.bottom;
                }

                if (cSpan.top > top)
                {
                    span.top = cSpan.top;
                }

                //1 is flagMergeDistance, when a walkable flag is favored before an unwalkable one
                if (AstarMath.Abs((int)span.top - (int)cSpan.top) <= voxelWalkableClimb)
                {
                    span.area = AstarMath.Max(span.area, cSpan.area);
                }

                VoxelSpan next = cSpan.next;
                if (prev != null)
                {
                    prev.next = next;
                }
                else
                {
                    firstSpan = next;
                }

                cSpan = next;
            }
        }

        if (prev != null)
        {
            span.next = prev.next;
            prev.next = span;
        }
        else
        {
            span.next = firstSpan;
            firstSpan = span;
        }
    }
}
~~~

#### VoxelMesh

~~~c#
/*
 * VoxelMesh used for recast graphs.
 */
public struct VoxelMesh
{
    public VInt3[] verts;
    public int[] tris;
    public uint[] layerFlag;
}
~~~



### 功能函数

#### Next

~~~c#
/** (i+1) % n assuming 0 <= i < n */
public static int Next(int i, int n)
{
    return i + 1 < n ? i + 1 : 0;
}
~~~

#### Prev

```c#
/** (i-1+n) % n assuming 0 <= i < n */
public static int Prev(int i, int n)
{
    return i - 1 >= 0 ? i - 1 : n - 1;
}
```

#### Diagonal

~~~c#
// 判断顶点 i 和 j 是否是多边形 P 的一个合适的内部对角线
public static bool Diagonal(int i, int j, int n, int[] verts, int[] indices)
{
    return InCone(i, j, n, verts, indices) && Diagonalie(i, j, n, verts, indices);
}
~~~

#### InCone

~~~c#
public static bool InCone(int i, int j, int n, int[] verts, int[] indices)
{
    int pi = (indices[i] & 0x0fffffff) * 4;
    int pj = (indices[j] & 0x0fffffff) * 4;
    int pi1 = (indices[Next(i, n)] & 0x0fffffff) * 4;
    int pin1 = (indices[Prev(i, n)] & 0x0fffffff) * 4;

    // If P[i] is a convex vertex [ i+1 left or on (i-1,i) ].
    if (LeftOn(pin1, pi, pi1, verts))
        return Left(pi, pj, pin1, verts) && Left(pj, pi, pi1, verts);

    // Assume (i-1,i,i+1) not collinear.
    // else P[i] is reflex.
    return !(LeftOn(pi, pj, pi1, verts) && LeftOn(pj, pi, pin1, verts));
}
~~~

#### Diagonalie

~~~c#
// 判断顶点 i 和 j 是否是多边形 P 的一个合适的内部或外部对角线，同时忽略与顶点 i 和 j 相关的边
static bool Diagonalie(int i, int j, int n, int[] verts, int[] indices)
{
    int d0 = (indices[i] & 0x0fffffff) * 4;
    int d1 = (indices[j] & 0x0fffffff) * 4;

    // 判断多边形 p 的每一条边（k，k+1）
    for (int k = 0; k < n; k++)
    {
        int k1 = Next(k, n);
        // 跳过与i，j相连的边
        if (!((k == i) || (k1 == i) || (k == j) || (k1 == j)))
        {
            int p0 = (indices[k] & 0x0fffffff) * 4;
            int p1 = (indices[k1] & 0x0fffffff) * 4;

            if (Vequal(d0, p0, verts) || Vequal(d1, p0, verts) || Vequal(d0, p1, verts) || Vequal(d1, p1, verts))
                continue;

            if (Intersect(d0, d1, p0, p1, verts))
                return false;
        }
    }
    return true;
}
~~~

#### Left

```c#
// 判断点 c 是否严格在由 a 到 b 的有向直线的左侧
public static bool Left(int a, int b, int c, int[] verts)
{
    return Area2(a, b, c, verts) < 0;
}
```

#### LeftOn

```c#
public static bool LeftOn(int a, int b, int c, int[] verts)
{
    int result = Area2(a, b, c, verts);
    return result <= 0;
}
```

#### Collinear

```c#
public static bool Collinear(int a, int b, int c, int[] verts)
{
    return Area2(a, b, c, verts) == 0;
}
```

#### Area2

```c#
// 叉乘运算，计算ab，ac两个向量叉乘（ab x ac）构成的平行四边形面积
// 正代表c点在ab边的右侧，负代表c点在ab边的左侧
public static int Area2(int a, int b, int c, int[] verts)
{
    return (verts[b] - verts[a]) * (verts[c + 2] - verts[a + 2]) - (verts[c + 0] - verts[a + 0]) * (verts[b + 2] - verts[a + 2]);
}
```

#### Vequal

```c#
static bool Vequal(int a, int b, int[] verts)
{
    return verts[a + 0] == verts[b + 0] && verts[a + 2] == verts[b + 2];
}
```

#### Between

```c#
// 判断一个点 c 是否在线段 ab 上，即点 c 在线段 ab 的闭合线段上
static bool Between(int a, int b, int c, int[] verts)
{
    // 如果不共线肯定不在
    if (!Collinear(a, b, c, verts))
        return false;
    
    // 如果线段 ab 不垂直于 x 轴，那么检查点 c 的 x 坐标是否在 a 和 b 的 x 坐标之间，如果是，则表示点 c 在线段 ab 的闭合线段上
    if (verts[a + 0] != verts[b + 0])
        return ((verts[a + 0] <= verts[c + 0]) && (verts[c + 0] <= verts[b + 0])) || ((verts[a + 0] >= verts[c + 0]) && (verts[c + 0] >= verts[b + 0]));
    // 如果线段 ab 垂直于 x 轴，那么检查点 c 的 y 坐标是否在 a 和 b 的 y 坐标之间，如果是，则表示点 c 在线段 ab 的闭合线段上
    else
        return ((verts[a + 2] <= verts[c + 2]) && (verts[c + 2] <= verts[b + 2])) || ((verts[a + 2] >= verts[c + 2]) && (verts[c + 2] >= verts[b + 2]));
}
```

#### Intersect

```c#
// 判断 ab 和 cd 两条线段是否相交，这里判断是否非严格相交，即如果一条线段的一点在另一条线段上也算相交
static bool Intersect(int a, int b, int c, int d, int[] verts)
{
    //严格相交直接返回true
    if (IntersectProp(a, b, c, d, verts))
        return true;
    //判断是否一条线段的一点在另一条线段上
    if (Between(a, b, c, verts) || Between(a, b, d, verts) || Between(c, d, a, verts) || Between(c, d, b, verts))
        return true;

    return false;
}
```

#### IntersectProp

```c#
//  判断两条线段 ab 和 cd 是否有合适的交点，即它们共享一个在两个线段内部的点
//  这个函数使用了严格的左侧判定（strict leftness）来确保交点的合适性
public static bool IntersectProp(int a, int b, int c, int d, int[] verts)
{
    // 如果ab，cd两个线段中有任何一个点与另一个线段共线则一定不相交
    if (Collinear(a, b, c, verts) || Collinear(a, b, d, verts) || Collinear(c, d, a, verts) || Collinear(c, d, b, verts))
        return false;

    //如果c,d两个点都在ab线段的同侧则一定不相交
    return Xorb(Left(a, b, c, verts), Left(a, b, d, verts)) && Xorb(Left(c, d, a, verts), Left(c, d, b, verts));
}
```

#### Xorb

只有一个参数为真时返回真，否则返回假

```c#
//  Xorb 方法可以确保只有一个参数为真时返回真，否则返回假
//  这个方法的实现灵感来自于 Michael Baldwin
public static bool Xorb(bool x, bool y)
{
    return !x ^ !y;
}
```



## 移除重叠顶点（RemoveOverlappedVerts）

~~~c#
public static void RemoveOverlappedVerts(ref VInt3[] verts, ref int[] tris, ref uint[] layerFlag)
{
    Dictionary<VInt3, int> dict = new Dictionary<VInt3, int>();
    int[] vertMap = new int[verts.Length];

    int vertIdx = 0;
    for (int i = 0; i < verts.Length; ++i)
    {
        VInt3 v = verts[i];
        int index;
        if (!dict.TryGetValue(v, out index))
        {
            dict.Add(v, vertIdx);
            index = vertIdx;
            vertIdx++;
        }

        vertMap[i] = index;
    }

    vertIdx = 0;
    verts = new VInt3[dict.Count];
    Dictionary<VInt3, int>.Enumerator it = dict.GetEnumerator();
    while (it.MoveNext())
    {
        verts[vertIdx++] = it.Current.Key;
    }

    for (int i = 0; i < tris.Length; ++i)
    {
        tris[i] = vertMap[tris[i]];
    }

    // 去重两个顶点合一的三角形
    List<int> newtris = new List<int>();
    List<uint> newLayerFlag = new List<uint>();

    for (int i = 0; i < tris.Length; i = i + 3)
    {
        int a = tris[i];
        int b = tris[i + 1];
        int c = tris[i + 2];

        if (a == b || b == c || a == c)
            continue;

        newtris.Add(a);
        newtris.Add(b);
        newtris.Add(c);

        if (layerFlag != null)
        {
            newLayerFlag.Add(layerFlag[i / 3]);
        }
    }

    tris = newtris.ToArray();

    if (layerFlag != null)
    {
        layerFlag = newLayerFlag.ToArray();
    }
}
~~~



# CPP构建

根据C#生成的信息构建出真正要使用的信息

## 构建入口

在Awake函数里对C#里的数据进行处理

~~~c++
void NavMesh::OnAwake()
{
#if SNAKE_TRACK
	hashCode_ = 0;
#endif//SNAKE_TRACK
	if (SGameDesk->Navigation)
	{
		if (bDynamicTemplate)
		{
			SGameDesk->Navigation->AddDynamicTemplateNavMesh(this);
		}
		else if (bBothMainAndExtra)
		{
			SGameDesk->Navigation->AddExtraNavMesh(this);
			SGameDesk->Navigation->AddNavMesh(this);
		}
		else if (bExtraNavMesh)
		{
			SGameDesk->Navigation->AddExtraNavMesh(this);
		}
		else if (bWallNavMesh)
		{
			SGameDesk->Navigation->AddWallNavMesh(this);
		}
		else
		{
			SGameDesk->Navigation->AddNavMesh(this);
		}
	}
	CreateNodeStructure();
	InitNodeConnectionCost();
	CreateEntity();
}
~~~

~~~c++
void NavMesh::CreateNodeStructure(bool forceCreate)
{
	if (!forceCreate && !NeedCreateNodeStrcuture())
	{
		return;
	}
	CreateNodes();
	CreateNodeConnections();
	CreateCellHandler();
	CreatePathNodes();
}
~~~



## 创建节点

### 类

#### NavNode

~~~c++
class NavNode
{
public:
	union
	{
		struct
		{
			uint32 v0_;
			uint32 v1_;
			uint32 v2_;
		};
		uint32 verts_[3];
	};

	VInt3 position_;
	uint32 index_;
	int32 connectedAreaIndex_;
	NavLayerFlag layerFlag_;
	uint8 influenceOrganCamp;
	TArrayLite<NavConnection, 24> connections_;
	//使点落在形状内部算法的搜索范围
	static const int32 InShapeShortSearchRadius = 1;
	static const int32 InShapeLongSearchRadius = 4;
	static const int32 InShapeMaxSearchRadius = 8;
public:
	NavNode();
	void Open(const NavMeshHandle& parent_, const PathHandle& path, const PathNodePtr& pathNode, const PathHandlerHandle& handler, NavLayerFlag layerFlag) const;
	bool SearchPointOpen(const NavMeshHandle& parent_, const PathHandle& path, const PathNodePtr& pathNode, const PathHandlerHandle& handler, int32 pathDistance, VInt3& result, ConstNavNodePtr& nearestNode, NavLayerFlag layerFlag) const;

	void UpdateG(const PathHandle& path, const PathNodePtr& pathNode) const;
	void UpdateRecursiveG(const NavMeshHandle& parent_, const PathHandle& path, const PathNodePtr& pathNode, const PathHandlerHandle& handler, NavLayerFlag layerFlag) const;
	bool ContainsConnection(const ConstNavNodePtr& node) const;
	bool ContainsConnection(const ConstNavNodePtr& node, uint32& cost) const;

	const VInt3& GetVertex(const NavMeshHandle& parent_, uint32 index) const;

	void GetVertices(const NavMeshHandle& parent_, VInt3& v0, VInt3& v1, VInt3& v2) const;
	void GetVertices(const NavMeshHandle& parent_, VInt3(&verts)[3]) const;

	bool GetPortal(const NavMeshHandle& parent_, const ConstNavNodePtr& other, TArray<VInt3>& left, TArray<VInt3>& right) const;
	bool GetPortal(const NavMeshHandle& parent_, const ConstNavNodePtr& other, TArray<VInt3>& left, TArray<VInt3>& right, bool backwards, int32& aIndex, int32& bIndex) const;

	ConstNavNodePtr GetNeighborByEdge(const NavMeshHandle& parent_, int32 edge, int32& otherEdge, SGCore::NavLayerFlag layerFlag) const;
	bool IsVertex(const NavMeshHandle& parent_, const VInt3& p, int32& index) const;

	int32 EdgeIntersects(const NavMeshHandle& parent_, const VInt3& a, const VInt3& b) const;
	int32 EdgeIntersects(const NavMeshHandle& parent_, VInt3 a, VInt3 b, int32 startEdge, int32 count) const;

	bool ContainsPointXZ(const NavMeshHandle& parent_, const VInt3& p) const;
	int32 GetColinearEdge(const NavMeshHandle& parent_, const VInt3& a, const VInt3& b) const;

	bool MakePointInTriangle(const NavMeshHandle& parent_, VInt3& result) const;
	bool MakePointInTriangle(const NavMeshHandle& parent_, VInt3& result, int32 minX, int32 maxX, int32 minZ, int32 maxZ, VInt3 offset) const;
	void MakePointInTriangleExact(const NavMeshHandle& parent_, VInt3& result, VInt3 offset) const;
	bool BelongLayer(NavLayerFlag layerFlag) const;

	SGAME_INLINE bool ExcludeLayer(NavLayerFlag layerFlag) const
	{
		return (layerFlag_ & layerFlag) == 0;
	}
	SGAME_INLINE bool ExcludeLayer(int32 layerFlag) const
	{
		return (layerFlag_ & (NavLayerFlag)layerFlag) == 0;
	}
private:
	static int32 FindAdjacentEdge(const uint32(&verts)[3], uint32 v0, uint32 v1);
};
~~~



### 功能函数

#### CreateNodes

~~~c++
//NavMesh类中的部分变量
TArray<VInt3> verts_;
TArray<int32> tris_;
TArray<VInt2> oneWay_;
TArray<uint32> layerFlags_;
uint32 layerCount_;
TArray<SGCore::NavNode> nodes_;
VRect bounds_;

//每个NavNode里存储三角形信息
void NavMesh::CreateNodes()
{
	verts_.Trim();
	tris_.Trim();
	connectedAreaLabel_.Trim();

	nodes_.Clear();
	nodes_.FillInit(tris_.Count() / 3); //每三个代表一个三角形，所以三角形数量 = tris的size除以3

	uint32 allLayerMark = 1;
	bool hasLayerFlags = layerFlags_.Count() > 0;		
	uint32 index = 0;
	for (uint32 i = 0; i < tris_.Count(); i += 3)
	{
        //因为tris里每三个代表一个三角形，所以每三个读取
		uint32 v0 = tris_[i + 0];
		uint32 v1 = tris_[i + 1];
		uint32 v2 = tris_[i + 2];

		NavNodePtr node = &nodes_[index];
		node->index_ = index;
        
        //保存三角形顶点的索引
		node->v0_ = v0;
		node->v1_ = v1;
		node->v2_ = v2;

        //node的位置用三角形中点表示
		node->position_ = verts_[v0];
		node->position_ += verts_[v1];
		node->position_ += verts_[v2];
		node->position_ /= 3;

		node->layerFlag_ = SGCore::NavLayerFlag0;  //layerflag默认值
		if (hasLayerFlags)
		{
			if (index < layerFlags_.Count())
			{
				node->layerFlag_ = (SGCore::NavLayerFlag)layerFlags_[index];  //layerFlag对应的是顶点形成的三角形区域，每个三角形对应一个flag
				allLayerMark |= node->layerFlag_;  //存储这个NavMesh里用到的所有layerflag
			}
		}

		node->connectedAreaIndex_ = -1;  //unexit label
		if (connectedAreaLabel_.Count() > 0)
		{
			if (index < connectedAreaLabel_.Count())
			{
				node->connectedAreaIndex_ = connectedAreaLabel_[index];
			}
		}

		index++;
	}

	layerCount_ = 0; //NavMesh类的变量
	for (uint32 i = 0; i < 32; ++i)
	{
		if ((allLayerMark & (1 << i)) != 0)
		{
			layerCount_++; //allLayerMark二进制位中只要是1就说明存在一个layer，多少个1就有多少个layer
		}
	}

    //根据各个顶点的位置来刷新Bound的区域
	bounds_.x = INT_MAX;
	bounds_.y = INT_MAX;
	bounds_.width = INT_MIN;
	bounds_.height = INT_MIN;

	for (uint32 i = 0; i < verts_.Count(); ++i)
	{
		_AddVertex(bounds_, verts_[i]);
	}

	bounds_.width -= bounds_.x;
	bounds_.height -= bounds_.y;
}
~~~



#### _AddVertex

```c++
void _AddVertex(VRect& bounds, const VInt3& p)
{
    if (p.x < bounds.x)
        bounds.x = p.x - 8;
    if (p.x > bounds.width)
        bounds.width = p.x + 8;

    if (p.z < bounds.y)
        bounds.y = p.z - 8;
    if (p.z > bounds.height)
        bounds.height = p.z + 8;
}
```



## 构建节点连接关系

根据三角形边的关系建立连接，因为Node已经存上了三个顶点对应的索引index，所以接下来就遍历所有的node，来根据index来创建边，然后判断共边性，就能判断连通性

### 结构体

#### NavConnection

```c++
struct NavConnection
{
    uint32 nodeIndex;
    struct
    {
        uint32 cost : 30;
        uint32 edge : 2;
    };
};
```



#### 功能函数

#### CreateNodeConnections

```c++
void NavMesh::CreateNodeConnections()
{
    TMap<uint64, uint32> edgeMap;
    edgeMap.Reserve(nodes_.Count() * 3);

    uint64 a, b;
    for (uint32 i = 0; i < nodes_.Count(); i++)
    {
       ConstNavNodePtr node = &nodes_[i];

       //遍历三角形的三条边
       for (uint32 j = 0; j < 3; ++j)
       {
          a = node->verts_[j];
          b = node->verts_[(j + 1) % 3];

          //每个边有一个唯一编码，编码方式是a<<32|b
          uint64 edge = a << 32 | b;
          if (!edgeMap.Contains(edge))
             edgeMap.Add(edge, i);
       }
    }

    //OnewayMap单独处理，逻辑同上
    TMap<uint64, uint32> oneWayMap;
    oneWayMap.Reserve(oneWay_.Count());
    for (uint32 i = 0; i < oneWay_.Count(); i++)
    {
       a = oneWay_[i].x;
       b = oneWay_[i].y;
       uint64 edge = a << 32 | b;
       if (!oneWayMap.Contains(edge))
          oneWayMap.Add(edge, i);
    }

    
    for (uint32 i = 0; i < nodes_.Count(); i++)
    {
       NavNodePtr node = &nodes_[i];
       for (uint32 j = 0; j < 3; ++j)
       {
          a = node->verts_[j];
          b = node->verts_[(j + 1) % 3];

          //这里编码方向相反是因为三角形假设是统一按顺时针存放的，那么它临近的三个三角形读取点的顺序一定和它相反，所以边向量相反
          uint64 edge = b << 32 | a;
          uint32 index = 0;

          //给单向节点添加临近一般节点
          if (oneWayMap.TryGet(index, edge))
          {
             uint32 oneWayAdjacentNode = 0;
             if (edgeMap.TryGet(oneWayAdjacentNode, edge))
             {
                oneWayNodeIndices_.Add(node->index_, oneWayAdjacentNode);
             }
             continue;
          }

          index = 0;
          if (edgeMap.TryGet(index, edge))
          {
             NavConnection conn;
             conn.edge = j;
                    
             VInt3 va = verts_[(int)a];
             VInt3 vb = verts_[(int)b];

              //叉乘计算node中点到ab边的投影
             VFactor f = IntMath::NearestPointFactorXZ(va, vb, node->position_);
             //vc是点position在边(va,vb)的投影点，cost则是中点position到边(va,vb）的距离
             VInt3 vc = (vb - va) * f + va;                                               
             uint32 cost2 = (uint32)(vc - node->position_).Magnitude();
             //g是从start到当前片的NavNode相连的cost之和，h是当前片重心到end的直线距离。可能出现 g值<h值的情况，导致寻路结果不是权值意义上的最优
             conn.cost = cost2;
                
             conn.nodeIndex = index;
             //保存和其他三角形连接关系的信息
             node->connections_.Add(conn);
          }
       }
    }
}
```



## 构建凸多边形

### 类

~~~c++
class NavMeshCellHandler : public PooledObject
{
	DECLARE_POOLED_CLASS;
public:

	struct EdgeIndex
	{
		int32 index0;
		int32 index1;

		EdgeIndex(int32 _index0, int32 _index1)
			: index0(_index0), index1(_index1)
		{

		}
	};
	struct Cell
	{
		TArrayLite<uint32> nodesIndex;

		TArrayLite<uint32> points;

		TArrayLite<EdgeIndex> ObstructEdges;

#if MEMORY_INSPECT
		Cell();
#endif
	};

	struct Edge
	{
		int32 x0, y0;
		int32 x1, y1;

		void Init(const VInt3& v0, const VInt3& v1);
	};


	TArray<Cell> cells_;

	int32 numX_;
	int32 numY_;

	int32 cellSize_;
	VInt2 origin_;

	NavMeshHandle parent_;

	TArray<uint32> cellIncludeOneWayNode_;

	  

	struct CellIntersection
	{
		ConstNavNodePtr node;
		int32 edge;
		int64 distance;
		VInt3 intersection;
	};

	
/*
     省略函数，具体内容直接查看代码文件
*/
};
~~~



### 功能函数

#### CreateCellHandler

~~~c++
void NavMesh::CreateCellHandler()
{
    //cellHandler可以理解为用于将整个Mesh划分为一个个tile并对每个tile进行处理的类
	if (!cellHandler_)
	{
		cellHandler_ = SGameDesk->GetHandle<NavMeshCellHandler>();
	}
	else
	{
		cellHandler_->Clear();
	}

	cellHandler_->parent_ = this;//setup parent first!
	cellHandler_->Init(bounds_, cellSize_); //cellSize_ 相当于tileSize，这里的初始化根据bounds的长和宽以及cellSize划分出一个个网格

	for (uint32 i = 0; i < nodes_.Count(); ++i)
	{
		cellHandler_->AddNode(&nodes_[i]);
	}

	cellHandler_->AddObstructEdge();
	cellHandler_->SortOneWayNode();

	TArray<NavMeshCellHandler::Cell>& cells = cellHandler_->cells_;
	for (uint32 i = 0; i < cells.Count(); ++i)
	{
		NavMeshCellHandler::Cell& curCell = cells[i];
		curCell.nodesIndex.Trim();
		curCell.points.Trim();
		curCell.ObstructEdges.Trim();
	}
	cells.Trim();
}
~~~

