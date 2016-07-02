#include <iostream>
#include <cstring>
using namespace std;

typedef struct
{
	const char * name;
	int number;
	char age;
}input_value;
class String
{
	private://私有的
   	    char *str;
	    input_value *siv;
	public://公开的
	    String(input_value *iv)
	    {
	    int length=strlen(iv->name);
	    length=sizeof(input_value);
	    siv =(input_value *)new char[length];
		cout << length<<endl;
	    str = new char[length+1];//申请内存
	    strcpy(str,iv->name);	
	    }
	    ~String()
	    {
		cout << "资源销毁\n" <<endl;
		delete[] str;    
	    }
	    void display()
	    {
		cout <<str<<endl;
	    }
};

int main(void)
{
	input_value iv;
	iv.name="namespace";
	String s1(&iv);
	s1.display();
	return 0;
}
