#include"type.h"

void u32BinaryAll(u32 x)
{
	u32 andx = 1;
	andx = andx << 31;
	int num = 0;
	while (andx)
	{
		if (andx & x)
			cout << "1";
		else
			cout << "0";
		num++;
		if (!(num & 0x7))
			cout << " ";
		andx = andx >> 1;
	}
}
void u32Binary(u32 x)
{
	u32 andx = 1;
	andx = andx << 31;
	int num = 0;
	while (!(andx&x))andx = andx >> 1;
	while (andx)
	{
		if (andx & x)
			cout << "1";
		else
			cout << "0";
		num++;
		if (!(num & 0x7))
			cout << " ";
		andx = andx >> 1;
	}
}

int main()
{
	while (1)
	{
		u32 x;
		cin >> x;
		u32Binary(x);
		cout << endl;
		u32BinaryAll(x);
		cout << endl;

	}
	return 0;

}

