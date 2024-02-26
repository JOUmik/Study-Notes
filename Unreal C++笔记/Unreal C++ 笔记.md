# Unreal C++ 笔记

[TOC]

## 1. C++中生成蓝图Actor

- StaticLoadClass（不推荐）

~~~c++
UClass* BlueprintVar = StaticLoadClass(AItem::StaticClass(), nullptr, TEXT(“蓝图路径”));
GetWorld()->SpawnActor<AItem>(BlueprintVar,&Location, &Rotation);
~~~

- TSubclassOf（推荐）

~~~c++
//.h
UPROPERTY(EditDefaultsOnly, Category = "CPPSettings|Projectile")
TSubclassOf<class AProjectile> ProjectileClass;

//.cpp
GetWorld()->SpawnActor<AItem>(ProjectileClass,&Location, &Rotation);
~~~





## 2.获取场景中的Actor

- ### 在蓝图中设置tag属性，通过`UGameplayStatics::GetAllActorsWithTag`查找

~~~c++
#include "Kismet/GameplayStatics.h"
TArray<AActor*> Actors;
UGameplayStatics::GetAllActorsWithTag(GetWorld(), TEXT("actorName"), Actors);

for (AActor* Actor: Actors){
}
~~~



-  ### 通过 `UGameplayStatics::GetAllActorsOfClass` 查找

~~~c++
#include "Kismet/GameplayStatics.h"
TArray<AActor*> Actors;
UGameplayStatics::GetAllActorsOfClass(GetWorld(), AActor::StaticClass(), Actors);
 
for (AActor* Actor : Actors){
}
~~~



## 3.OnHit

~~~c++
UFUNCTION()
void OnHit(UPrimitiveComponent* HitComponent, AActor* OtherActor, UPrimitiveComponent* OtherComp, FVector NormalImpulse, const FHitResult& Hit)
~~~



## 4.OnBeginOverlap

~~~c++
UFUNCTION()
void OnOverlapBegin(UPrimitiveComponent* OverlappedComponent, AActor* OtherActor, UPrimitiveComponent* OtherComp, int32 OtherBodyIndex, bool bFromSweep, const FHitResult& SweepResult)

//一定要放在BeginPlay里
void BeginPlay(){
	Mesh->OnComponentBeginOverlap.AddDynamic(this, &AItem::OnOverlapBegin);   
}
~~~



## 5.OnEndOverlap

~~~c++
UFUNCTION()
void OnOverlapEnd(UPrimitiveComponent* OverlappedComponent, AActor* OtherActor, UPrimitiveComponent* OtherComp, int32 OtherBodyIndex)
    
//一定要放在BeginPlay里
void BeginPlay(){
	Mesh->OnComponentEndOverlap.AddDynamic(this, &AItem::OnOverlapEnd);   
}
~~~



## 6.Output Log

- 打印在输出日志

~~~c++
UE_LOG(LogTemp, Display, TEXT("Your message"));
~~~

~~~c++
FString s = "this is a string";
UE_LOG(LogTemp, Display, TEXT("Here's My String: %s"), *s);
~~~

- 打印在屏幕窗口

~~~c++
FString s = "this is a string";
GEngine->AddOnScreenDebugMessage(-1, 5.f, FColor::Green, FString::Printf(TEXT("Successed Produce NPC : %s"), *s));
~~~

~~~c++
#include "Kismet/KismetSystemLibrary.h"

FString s = "this is a string";
UKismetSystemLibrary::PrintString(this, (TEXT("Successed Produce NPC:%s"), *s), true, true, FColor::Blue, 5.f);
~~~



## 7.VInterpConstantTo

在两个位置之间插值

~~~c++
#include "Math/UnrealMathUtility.h"

FVector NewLocation = FMath::VInterpConstantTo(CurrentLocation, TargetLocation, DeltaTime, Speed);
~~~



## 8.Forward Declaration(正向声明)

通过正向声明可以避免在.h文件中include过多库文件，通过在Cpp中引用库文件防止编译时间过长

~~~c++
UPROPERTY()
class UCapsuleComponent* CapsuleComp;
~~~



## 9.UPROPERTY

- **UPROPERTY(VisibleAnywhere)**：细节面板可见，但是无法编辑
- **UPROPERTY(VisibleInstanceOnly)**：只有场景中对应的实例可见，无法编辑
- **UPROPERTY(VisibleDefaultOnly)**：只有蓝图中可见，无法编辑
- **UPROPERTY(EditAnywhere)**：细节面版可见，且可编辑
- **UPROPERTY(EditInstanceOnly)**：只有场景中对应的实例可见，可编辑
- **UPROPERTY(EditDefaultOnly)**：只有蓝图中可见，可编辑

- **UPROPERTY(BlueprintReadWrite)**：与蓝图的Event有关，可以在蓝图中读取并设置
- **UPROPERTY(BlueprintReadOnly)**：与蓝图的Event有关，可以在蓝图中读取但不能设置
- **UPROPERTY(BlueprintReadWrite, meta = (AllowPrivateAccess  =  "true"))**：允许为Private变量添加BlueprintReadWrite属性
- **UPROPERTY(EditAnywhere， Category = “Custom Name”)**：细节面版可见（放在自己自定义的Category中），且可编辑



## 10.枚举

~~~c++
UENUM(BlueprintType)
enum class EGamePhase : uint8 {
	Intro     UMETA(DisplayName = "Intro"),   //第一项必须设为0
	Wait = 2     UMETA(DisplayName = "Wait"),
	Draw = 3     UMETA(DisplayName = "Draw"),
	Finished  UMETA(DisplayName = "Finished")
};
~~~



## 11. FRandomStream

~~~c++
#include "Math/RandomStream.h"

random = FRandomStream();
WaitTime = random.GetFraction() * (WaitMaxTime - WaitMinTime) + WaitMinTime;
~~~



## 12. UClass和Actor的获取

为了获取场景中的Actor，一般都需要传入UClass，UClass代表着我们想要获取的Actor的class，对于蓝图类和c++类，有着不同的获取方式：

- C++类：

~~~c++
#include "Kismet/GameplayStatics.h"

//UClass* 变量名 = 想要获取的类名::StaticClass()
UClass* Exclamation = AExclamation::StaticClass();  

ExclamActor = Cast<AExclamation>(UGameplayStatics::GetActorOfClass(GetWorld(), Exclamation));
~~~

- 蓝图类：

~~~c++
//UClass* 变量名 = StaticLoadClass(想要获取的蓝图类的基类::StaticClass(), nullptr, TEXT(想要获取的蓝图类的引用路径))
UClass* BlueprintVar = StaticLoadClass(
    ASpawnActor::StaticClass(), 
    nullptr, 
    TEXT("/Script/Engine.Blueprint'/Game/Blueprints/SpawnActorFromSky.SpawnActorFromSky_C'")
);

TArray<AActor*> Actors;
UGameplayStatics::GetAllActorsOfClass(GetWorld(), BlueprintVar, Actors);
~~~



## 13. 音乐

~~~c++
#include "Kismet/GameplayStatics.h"
#include "Sound/SoundCue.h"

USoundCue* FootSound;

if (FlipBookComp->GetPlaybackPositionInFrames() == 1 || FlipBookComp->GetPlaybackPositionInFrames() == 5) {
	if (!SoundHasPlayInFrame) {
		UGameplayStatics::PlaySound2D(GetWorld(), FootSound);
		SoundHasPlayInFrame = true;
	}
}
else {
	SoundHasPlayInFrame = false;
~~~



## 14. Create Components

在C++类的Constructor中，可以使用CreateDefaultSubobject生成组件并使用SetUpAttachment绑定组件：

~~~c++
ABackground::ABackground()
{
 	// Set this actor to call Tick() every frame.  You can turn this off to improve performance if you don't need it.
	PrimaryActorTick.bCanEverTick = true;
	SceneComp = CreateDefaultSubobject<USceneComponent>(TEXT("RootComponent"));
	RootComponent = SceneComp;
    
    BoxComp = CreateDefaultSubobject<UBoxComponent>(TEXT("Box Collision"));
	BoxComp->SetupAttachment(RootComponent);
}
~~~

如果要在其他函数，如生命周期函数OnConstruction中生成组件，要使用NewObject； 绑定组件需使用AttachToComponent：

~~~c++
void ABackground::OnConstruction(const FTransform& Transform) {
	Super::OnConstruction(Transform);

	for (int i = 0; i < Num; i++) {
		FString  s = "Background " + FString::FromInt(i);
		if (BackgroundSprite) {
			UPaperSpriteComponent* PaperComp;
			PaperComp = NewObject<UPaperSpriteComponent>(this, UPaperSpriteComponent::StaticClass(), FName(*s));
			PaperComp->RegisterComponent();
			PaperComp->SetSprite(BackgroundSprite);
			PaperComp->AttachToComponent(RootComponent, FAttachmentTransformRules::KeepRelativeTransform);
			PaperComp->SetRelativeLocation(FVector(PaperComp->GetRelativeLocation().X + BackgroundLength * i, PaperComp->GetRelativeLocation().Y, PaperComp->GetRelativeLocation().Z));
		}
	}
}
~~~



## 15. C++模板

### (1) Actor

- .h file

~~~c++
protected:
	virtual void BeginPlay() override;

public:
	APlatformChar();

	//virtual void OnConstruction(const FTransform& Transform) override;
	
	// Called every frame
	virtual void Tick(float DeltaTime) override;

	virtual void SetupPlayerInputComponent(class UInputComponent* PlayerInputComponent) override;
private:

~~~



- .cpp file

~~~c++
APlatformChar::APlatformChar() {
	PrimaryActorTick.bCanEverTick = true;
}

void APlatformChar::BeginPlay() {
	Super::BeginPlay();
}

void APlatformChar::Tick(float DeltaTime) {
	Super::Tick(DeltaTime);
}

void APlatformChar::SetupPlayerInputComponent(UInputComponent* PlayerInputComponent) {
	Super::SetupPlayerInputComponent(PlayerInputComponent);

}
~~~



#### A. Enhanced Input

- .h file

~~~c++
class UInputMappingContext;
class UInputComponent;
class UInputAction;
struct FInputActionValue;

public:
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "CPPSettings|Input Setting")
	UInputMappingContext* MappingContext;

private:
	//Input Action
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "CPPSettings|Input Setting", meta = (AllowPrivateAccess = "true"))
	class UInputAction* MoveAction;

	void Move(const FInputActionValue& Value);
~~~



- .cpp file

~~~c++
#include "EnhancedInputComponent.h"
#include "EnhancedInputSubsystems.h"
#include "InputActionValue.h"
#include "Engine/LocalPlayer.h"

void APlatformChar::BeginPlay() {
	Super::BeginPlay();

	if (APlayerController* PlayerController = Cast<APlayerController>(GetController())) {
		if (UEnhancedInputLocalPlayerSubsystem* Subsystem = ULocalPlayer::GetSubsystem<UEnhancedInputLocalPlayerSubsystem>(PlayerController->GetLocalPlayer())) {
			Subsystem->AddMappingContext(MappingContext, 0);
		}
	}
}

void APlatformChar::SetupPlayerInputComponent(class UInputComponent* PlayerInputComponent) {
	Super::SetupPlayerInputComponent(PlayerInputComponent);

	UEnhancedInputComponent* InputComp = Cast<UEnhancedInputComponent>(PlayerInputComponent);
	if (InputComp) {
		InputComp->BindAction(MoveAction, ETriggerEvent::Triggered, this, &APlatformChar::Move);
	}
}

void APlatformChar::Move(const FInputActionValue& Value){
    
}
~~~



#### B. Components Setting

- .h file

~~~c++
//Components

//BoxComponent
UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Components", meta = (AllowPrivateAccess = "true"))
class UBoxComponent* BoxComp;

//Camera
UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Components", meta = (AllowPrivateAccess = "true"))
class USpringArmComponent* SpringArm;
UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Components", meta = (AllowPrivateAccess = "true"))
class UCameraComponent* Camera;
~~~



.cpp file

~~~c++
#include "GameFramework/SpringArmComponent.h"
#include "Camera/CameraComponent.h"
#include "Components/BoxComponent.h"

AKing::AKing() {
	PrimaryActorTick.bCanEverTick = true;

    //Box Component
    BoxComp = CreateDefaultSubobject<UBoxComponent>(TEXT("Box Collision"));
    
    BoxComp->SetupAttachment(RootComponent);
    
    //Camera
	SpringArm = CreateDefaultSubobject<USpringArmComponent>(TEXT("Spring Arm"));
	Camera = CreateDefaultSubobject<UCameraComponent>(TEXT("Camera"));
    
	SpringArm->SetupAttachment(RootComponent);
	Camera->SetupAttachment(SpringArm);
}
~~~



### (2) Component

~~~c++
//后续添加
~~~



## 16. TimerManager

- 方法一：回调函数不需要传参

.h file

~~~c++
FTimerHandle FireRateTimeHandle;
float FireRate = 2.f;
void CheckFireCondition();
~~~



.cpp file

~~~c++
#include "TimerManager.h"

GetWorldTimerManager().SetTimer(FireRateTimeHandle, this, &ATower::CheckFireCondition, FireRate, true);
~~~



- 方法二：回调函数需要传参

.h file

~~~c++
FTimerHandle StartGameTimerHandle;
float StartDelay = 3.f;
~~~



.cpp file

~~~c++
//使用计时器委托传入需要的回调函数及对应参数
FTimerDelegate StartGameTimerDelegate = FTimerDelegate::CreateUObject(
    PlayerController, 
    &AToonTanksPlayerController::SetPlayerEnabledState, 
    true
);
GetWorldTimerManager().SetTimer(
    StartGameTimerHandle, 
    StartGameTimerDelegate, 
    StartDelay, 
    false
);
~~~

更多用法请参考官方文档：

[FTimerManager | Unreal Engine 5.2 Documentation](https://docs.unrealengine.com/5.3/en-US/API/Runtime/Engine/FTimerManager/)



## 17. ApplyDamage

受到伤害的Actor或Comp.h

~~~c++
UFUNCTION()
void DamagedTaken(AActor* DamagedActor, float Damage, const UDamageType* DamageType, class AController* Instigator, AActor* DamageCauser)
~~~



受到伤害的Actor或Comp.cpp

~~~c++
//Comp
GetOwner()->OnTakeAnyDamage.AddDynamic(this, &UHealthComponent::DamagedTaken);

//Actor
OnTakeAnyDamage.AddDynamic(this, &UHealthComponent::DamagedTaken);
~~~



造成伤害的Actor对应cpp：

~~~c++
//以子弹的OnHit为例
//击中后调用ApplyDamage，此时对应Actor与OnTakeAnyDanage绑定的回调函数会被调用
void AProjectile::OnHit(UPrimitiveComponent* HitComponent, AActor* OtherActor, UPrimitiveComponent* OtherComp, FVector NormalImpulse, const FHitResult& Hit) {
	auto MyOwner = GetOwner();

	if (MyOwner) {
		auto MyOwnerInstigator =  MyOwner->GetInstigatorController();
		UClass* DamageTypeClass = UDamageType::StaticClass();

		if (OtherActor && OtherActor != this && OtherActor != MyOwner) {
			UGameplayStatics::ApplyDamage(OtherActor, Damage, MyOwnerInstigator, this, DamageTypeClass);
		}
	}
	Destroy();
}
~~~



## 18. Trace

### (1) BoxTrace

~~~c++
const FVector StartPoint = BoxTraceStartPoint->GetComponentLocation();
const FVector EndPoint = BoxTraceEndPoint->GetComponentLocation();

TArray<AActor*> ActorsToIgnore;
FHitResult BoxHitResult;
	
UKismetSystemLibrary::BoxTraceSingle(
	this,
	StartPoint,
	EndPoint,
	FVector(5.f, 5.f, 5.f),
	BoxTraceStartPoint->GetComponentRotation(),
	ETraceTypeQuery::TraceTypeQuery1,
	false,
	ActorsToIgnore,
	EDrawDebugTrace::Type::ForDuration,
	BoxHitResult,
	true
	);
~~~



## 19. Collision

~~~c++
GetMesh()->SetCollisionEnabled(ECollisionEnabled::NoCollision);
GetMesh()->SetCollisionObjectType(ECC_WorldDynamic);
GetMesh()->SetCollisionResponseToAllChannels(ECR_Overlap);
GetMesh()->SetCollisionResponseToChannel(ECC_Pawn, ECR_Ignore);
GetMesh()->SetGenerateOverlapEvents(true);
~~~



## 20. Interface

- **Determine Whether a Class Implements an Interface**

~~~c++
bool bIsImplemented;

/* bIsImplemented is true if OriginalObject implements UReactToTriggerInterface */
bIsImplemented = OriginalObject->GetClass()->ImplementsInterface(UReactToTriggerInterface::StaticClass());

/* bIsImplemented is true if OriginalObject implements UReactToTriggerInterface */
bIsImplemented = OriginalObject->Implements<UReactToTriggerInterface>();

/* ReactingObject is non-null if OriginalObject implements UReactToTriggerInterface in C++ */
IReactToTriggerInterface* ReactingObject = Cast<IReactToTriggerInterface>(OriginalObject);
~~~



## 21. Montage

~~~c++
void ASlashCharacter::PlayAttackMontage()
{
	if(UAnimInstance* AnimInstance = GetMesh()->GetAnimInstance())
	{
		if(AttackMontage)
		{
			AnimInstance->Montage_Play(AttackMontage);

			FName SectionName = FName();
			SectionName = FName("Attack1");

			AnimInstance->Montage_JumpToSection(SectionName, AttackMontage);
		}
		
	}
}
~~~



## 22. PlaySound

~~~c++
#include "Kismet/GameplayStatics.h"

if(HitSound)
{
	UGameplayStatics::PlaySoundAtLocation(
		this,
		HitSound,
		ImpactPoint
		);
}
~~~



## 23. VFX

### (1) Cascade

~~~c++
.h
UPROPERTY(EditAnywhere, Category= "CPP Settings|VFX")
UParticleSystem* HitParticle;

.cpp
if(HitParticle)
{
	UGameplayStatics::SpawnEmitterAtLocation(
		GetWorld(),
		HitParticle,
		ImpactPoint
	);
}
~~~



### (2) Niagram



## 24. DrawDebug

### (1) DrawDebugArrow

~~~c++
UKismetSystemLibrary::DrawDebugArrow(
	this,
	GetActorLocation(),
	GetActorLocation()+ToHit*80,
	5.f,
	FColor::Purple,
	5.f
);
~~~



### (2) DrawDebugSphere

~~~c++
#include "DrawDebugHelpers.h"

DrawDebugSphere(
	GetWorld(), 
	Location, 
	25.f, 
	12,
	FColor::Red, 
	true
);
~~~



