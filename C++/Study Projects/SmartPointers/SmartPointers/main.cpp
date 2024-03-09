#include <memory>

#include "Cat.h"

void show_unique_ptr();
void UniquePointerPassValue(unique_ptr<Cat> c);
void UniquePointerPassRef(unique_ptr<Cat> &c);
void UniquePointerPassConstRef(const unique_ptr<Cat>& c);

void show_shared_ptr();
void SharedPointerPassValue(shared_ptr<Cat> c);
void SharedPointerPassRef(shared_ptr<Cat>& c);
void SharedPointerPassConstRef(const shared_ptr<Cat>& c);

unique_ptr<Cat> GetUniquePtr();
void Transfer();


void show_weak_ptr();

int main() {
	show_unique_ptr();
	cout << endl;
	show_shared_ptr();
	cout << endl;
	Transfer();
	show_weak_ptr();

	return 0;
}

//展示独占指针的三种使用方式
void show_unique_ptr() {
	
	cout << "----------------unique_ptr----------------" << endl;
	//方法一
	cout << "<method 1>" << endl;
	Cat* c_p1 = new Cat("c_p1");
	unique_ptr<Cat> c_up1(c_p1);
	c_up1->CatInfo();
	c_p1 = nullptr;
	delete c_p1;
	cout << endl;

	//方法二
	cout << "<method 2>" << endl;
	unique_ptr<Cat> c_up2{ new Cat("c_up2") };
	unique_ptr<int> i_up2{ new int(100) };
	c_up2->CatInfo();
	cout << *i_up2 << endl;
	cout << "cat address: " << c_up2.get() << endl;
	cout << "int address: " << i_up2.get() << endl;
	cout << endl;

	//方法三：std::make_unique
	cout << "<method 3>" << endl;
	unique_ptr<Cat> c_up3 = make_unique<Cat>("c_up3");
	unique_ptr<int> i_up3 = make_unique<int>(200);
	c_up3->CatInfo();
	cout << *i_up3 << endl;
	cout << "cat address: " << c_up3.get() << endl;
	cout << "int address: " << i_up3.get() << endl;
	cout << endl;

	//独占指针不能copy，只能move，要注意所有权问题
	cout << "<Function Call: Pass Value>" << endl;
	unique_ptr<Cat> c_PassValue = make_unique<Cat>("c_PassValue");
	cout << endl;
	UniquePointerPassValue(move(c_PassValue));
	cout << endl;
	UniquePointerPassValue(make_unique<Cat>());
	cout << endl;
	//c_PassValue->CatInfo();   //不能再调用，因为指针指向内容的所有权已经移交给DoWithCatPassValue

	//Pass Reference with no const
	cout << "<Function Call: Pass Ref>" << endl;
	unique_ptr<Cat> c_PassRef = make_unique<Cat>("c_PassRef");
	cout << endl;
	UniquePointerPassRef(c_PassRef);
	cout << endl;

	//Pass Reference with const
	cout << "<Function Call: Pass Const Ref>" << endl;
	unique_ptr<Cat> c_PassConstRef = make_unique<Cat>("c_PassConstRef");
	cout << endl;
	UniquePointerPassConstRef(c_PassConstRef);
	cout << endl;
	cout << "cat address: " << c_PassConstRef.get() << endl;
	cout << endl;

	cout << "------------>结束<------------" << endl;
	cout << "----->智能指针自动delete<-----" << endl;
}

void UniquePointerPassValue(unique_ptr<Cat> c) {
	cout << "-----Pass Value-----" << endl;
	c->CatInfo();

	cout << "------>Finish<------" << endl;
}
void UniquePointerPassRef(unique_ptr<Cat>& c) {
	cout << "------Pass Ref------" << endl;
	c->SetCatName("c_PassRef_Rename");
	c->CatInfo();
	c.reset();
	cout << "------>Finish<------" << endl;
}
void UniquePointerPassConstRef(const unique_ptr<Cat>& c) {
	cout << "---Pass Const Ref---" << endl;
	c->SetCatName("c_PassConstRef_Rename");
	c->CatInfo();
	cout << "------>Finish<------" << endl;
}


void show_shared_ptr() 
{
	cout << "----------------shared_ptr----------------" << endl;
	cout << "<< CONSTANT TYPE >>" << endl;
	shared_ptr<int> i_sp1 = make_shared<int>(10);
	cout << "i_sp1 value: " << *i_sp1 << endl;
	cout << "i_sp1 use count: " << i_sp1.use_count() << endl;
	shared_ptr<int> i_sp2 = i_sp1;  //copy
	cout << "(copy i_sp1, create i_sp2)" << endl;
	cout << "i_sp2 value: " << *i_sp2 << endl;
	cout << "i_sp1 use count: " << i_sp1.use_count() << endl;
	cout << "i_sp2 use count: " << i_sp2.use_count() << endl;
	cout << "(change i_sp2 value, both change)" << endl;
	*i_sp2 += 5;
	cout << "i_sp2 new value: " << *i_sp2 << endl;
	cout << "i_sp1 new value: " << *i_sp1 << endl;
	cout << "(reset i_sp2)" << endl;
	i_sp2.reset();
	cout << "i_sp1 value: " << *i_sp1 << endl;
	cout << "i_sp1 use count: " << i_sp1.use_count() << endl;
	cout << "i_sp2 use count: " << i_sp2.use_count() << endl;
	cout << "(copy i_sp1, create i_sp3)" << endl;
	shared_ptr<int> i_sp3 = i_sp1;  //copy
	cout << "i_sp1 use count: " << i_sp1.use_count() << endl;
	cout << "i_sp2 use count: " << i_sp2.use_count() << endl;
	cout << "i_sp3 use count: " << i_sp3.use_count() << endl;
	cout << "(reset i_sp1)" << endl;
	i_sp1.reset();
	cout << "i_sp3 value: " << *i_sp3 << endl;
	cout << "i_sp1 use count: " << i_sp1.use_count() << endl;
	cout << "i_sp2 use count: " << i_sp2.use_count() << endl;
	cout << "i_sp3 use count: " << i_sp3.use_count() << endl;
	cout << endl;

	cout << "<< CUSTOM TYPE >>" << endl;
	shared_ptr<Cat> c_sp1 = make_shared<Cat>("c_sp1");
	c_sp1->CatInfo();
	cout << "c_sp1 use count: " << c_sp1.use_count() << endl;
	cout << "(copy c_sp1, create c_sp2, c_sp3)" << endl;
	shared_ptr<Cat> c_sp2 = c_sp1;
	shared_ptr<Cat> c_sp3 = c_sp1;
	cout << "c_sp1 use count: " << c_sp1.use_count() << endl;
	cout << "c_sp2 use count: " << c_sp2.use_count() << endl;
	cout << "c_sp3 use count: " << c_sp3.use_count() << endl;
	cout << "(reset all, automatically delete)" << endl;
	c_sp1.reset();
	c_sp2.reset();
	c_sp3.reset();
	cout << endl;

	cout << "<< FUNCTION CALL: PASS VALUE >>" << endl;
	shared_ptr<Cat> c_PassValue = make_shared<Cat>("c_PassValue");
	cout << endl;
	SharedPointerPassValue(c_PassValue);
	cout << endl;
	c_PassValue->CatInfo();
	cout << "c_PassValue use count: " << c_PassValue.use_count() << endl;
	cout << endl;

	cout << "<< FUNCTION CALL: PASS REF >>" << endl;
	shared_ptr<Cat> c_PassRef = make_shared<Cat>("c_PassRef");
	cout << endl;
	SharedPointerPassRef(c_PassRef);
	cout << endl;
	c_PassValue->CatInfo();
	cout << endl;


	cout << "<< FUNCTION CALL: PASS CONST REF >>" << endl;
	shared_ptr<Cat> c_PassConstRef = make_shared<Cat>("c_PassConstRef");
	cout << endl;
	SharedPointerPassConstRef(c_PassConstRef);
	cout << endl;

	cout << "------------>结束<------------" << endl;
	cout << "----->智能指针自动delete<-----" << endl;
}

void SharedPointerPassValue(shared_ptr<Cat> c)
{
	cout << "-----Pass Value-----" << endl;
	cout << "(Func change name to c_PassValue_Rename)" << endl;
	c->SetCatName("c_PassValue_Rename");
	cout << "Func use count: " << c.use_count() << endl;
	cout << "------>Finish<------" << endl;
	
}
void SharedPointerPassRef(shared_ptr<Cat>& c)
{
	cout << "------Pass Ref------" << endl;
	c->CatInfo();
	cout << "(Reset to a new cat)" << endl;
	c.reset(new Cat());
	cout << "Func use count: " << c.use_count() << endl;
	cout << "------>Finish<------" << endl;
}
void SharedPointerPassConstRef(const shared_ptr<Cat>& c)
{
	cout << "---Pass Const Ref---" << endl;
	c->CatInfo();
	cout << "Func use count: " << c.use_count() << endl;
	cout << "------>Finish<------" << endl;
}


unique_ptr<Cat> GetUniquePtr() {
	unique_ptr<Cat> c = make_unique<Cat>("local cat");
	return c;
}
void Transfer() {

	cout << "-----------------transfer-----------------" << endl;
	unique_ptr<Cat> c_up = make_unique<Cat>("c_up");
	shared_ptr<Cat> c_sp = move(c_up);

	cout << "c_sp use count: " << c_sp.use_count() << endl;

	//func unique->shared(right reference), cannot tranfer shared to unique
	shared_ptr<Cat> c_sp1 = GetUniquePtr();
	if (c_sp1) 
	{
		c_sp1->CatInfo();
		cout << "c_sp1 use count: " << c_sp1.use_count() << endl;
	}

	cout << endl;
	cout << "------------>结束<------------" << endl;
	cout << "----->智能指针自动delete<-----" << endl;
}


void show_weak_ptr() {
	//weak_ptr不能调用->和解引用*
	//防止使用shared_ptr导致的循环依赖问题(两个类型实例的变量中都保存了对方，销毁时会循环销毁)
	//weak_ptr可以使用lock提升为shared_ptr
	cout << "-----------------weak_ptr-----------------" << endl;

	shared_ptr<Cat> c_sp = make_shared<Cat>("c_sp");
	weak_ptr<Cat> c_wp(c_sp);

	cout << "c_wp use count: " << c_wp.use_count() << endl;
	cout << "c_sp use count: " << c_sp.use_count() << endl;
	cout << "(Lock to shared_ptr)" << endl;
	shared_ptr<Cat> c_sp1 = c_wp.lock();
	cout << "c_wp use count: " << c_wp.use_count() << endl;
	cout << "c_sp use count: " << c_sp.use_count() << endl;
	cout << "c_sp1 use count: " << c_sp1.use_count() << endl;
	 
	cout << endl;
	cout << "------------>结束<------------" << endl;
	cout << "----->智能指针自动delete<-----" << endl;
}