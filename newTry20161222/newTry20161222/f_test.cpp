#include"f_test.h"
int fileOpenTest(char *fileName)
{
	ifstream in;
	in.open("management-server.log");
	if (!in.is_open())
	{
		cout << "open file error!" << endl;
		return -1;
	}
	char buff[100];
	in.read(buff, 100);
	for (int i = 0; i < 100; i++)
		cout << buff[i];
	cout << endl;
	cout << "------test success------" << endl;
	return 0;
}
int testTellg(char * fileName)
{
	ofstream out;
	out.open(fileName, ios::out);
	char *str = "i think i love you very much,and i hope you have the same motion as me!/";
	out.write(str, strlen(str));
	cout<<"tellp="<<out.tellp()<<endl;
	out.close();
	ifstream in;
	in.open(fileName, ios::in);

	while (1)
	{
		char ch_t[3];
		in.read(ch_t, 2);
		ch_t[2] = '\0';
		cout << ch_t << "  tellg=" << in.tellg() << "  count=";
		cout << in.gcount() << endl;

	}
	in.close();
}