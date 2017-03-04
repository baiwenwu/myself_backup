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
#if 0
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
#endif
#if 0
double recursiveSum(double a1, double an, double step)
{

	if (an >= a1){	
		double tmp = recursiveSum(a1, an - step, step) + an;
		return tmp;
	}
	return 0;
}
int digitSum(int a1, int an,int step)
{
	int sum = 0;
	for (int i = a1; i < an + 1;)
	{
		int tmp = i;
		while (tmp)
		{
			sum += tmp % 10;
			tmp = tmp / 10;
		}
		i += step;
	}
	return sum;
}
int sumOneNumber(int R, int S, int L,int step)
{
	int tmp1 = R*step * 45;
	int tmp2 = recursiveSum(1, S - 1, 1) * 10;
	int tmp3 = S*L;
	return tmp1 + tmp2 + tmp3 + S;
}
int formulaDigitSum(int x)
{
	int step = 10;
	int sum = 0;
	//个位
	int num = x / 10;
	sum += num * 45;
	int remainder = x % 10;
	sum += recursiveSum(1, remainder, 1);

	//十位
	int L = remainder;
	x = x / 10;
	while (x)
	{
		int R = x / 10;
		int S = x % 10;
		sum+=sumOneNumber(R, S, L, step);
		L = S;
		x = x / 10;
		step *= 10;
	}
	return sum;
}

int main()
{
	double x=1, y=123;
	//cin >> x >> y;
	//cout<<recursiveSum(x, y, 1);
	for (int i = 101; i < 1000; i++)
	{
		int tmp = digitSum(x, i, 1);
		int tmp2 = formulaDigitSum(i);
		if (tmp != tmp2)
		{
			cout << "er!" << endl;
		}
	}
	cout << digitSum(x, y, 1);
	
	return 0;
}
#endif