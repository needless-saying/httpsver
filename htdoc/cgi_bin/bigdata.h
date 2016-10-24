#pragma once
#include<iostream>
#include<assert.h>
#include<string>
using namespace std;

#define UN_INIT 0XCCCCCCCCCCCCCCCCLL;
#define MAX 0x7FFFFFFFFFFFFFFFLL
#define MIN 0X8000000000000000LL
typedef long long  INT64;

class BigData
{
public:
	BigData(INT64 value=0xCCCCCCCCCCCCCCCCLL) :_value(value)
	{
		ToString();
	};
	BigData(const char *p);
	BigData operator+(BigData & bigdata);
	BigData operator-(BigData & bigdata);
	BigData operator*(BigData & bigdata);
	BigData operator/(BigData & bigdata);

	friend ostream& operator <<(ostream & _cout, const BigData& bigdata);
private:
	bool IsOver64()const
	{
		string tmp("+9223372036854775807");
 		if (_StrData[0] == '-')
		{
			tmp = "-9223372036854775808";
		}
		if (_StrData.size() < tmp.size())
			return true;
		else if (_StrData.size() == tmp.size() && _StrData <= tmp)
			return true;
		return false;
	}
	string Add(string left, string right);
	string Sub(string left, string right);
	string Mul(string left, string right);
	string Div(string left, string right);


private:
	bool IsLeftBig(const char *left, int lSize, const char *right, int rSize);
	char SubLoop( char *left, int lSize, const char *right, int rSize);
	void ToString();
	INT64 _value;
	string _StrData;
};
BigData::BigData(const char *pData)
{
	//1212312   asdasd     1221312qwe     0000qweqwe
	if (pData == NULL)
		return;
	char *p =(char *) pData;
	char _Symble = pData[0];
	if (_Symble == '+'||_Symble == '-')
	{
		p++;
	}
	else if (_Symble >= '0' && _Symble <= '9')
	{
		_Symble = '+';
	}
	else _Symble = '-';
	while (*p == '0')
	{
		p++;
	}
	_StrData.resize(strlen(pData) + 1);
	_StrData[0] = _Symble;
	int count = 1;
	 _value = 0;
	while (*p>='0'&&*p<='9')
	{
		_value = _value*10 + (*p-'0');
		_StrData[count++] = *p;
		p++;
	}
	if (_Symble == '-')
		_value = 0 - _value;
	_StrData.resize(count);
};
 ostream&  operator<<(ostream & _cout, const BigData& bigdata)
{
	 //是否溢出判断
	 //
	 if (bigdata.IsOver64())
	 {
		 _cout << bigdata._value << endl;
	 }
	 else _cout << bigdata._StrData << endl;
	 return _cout;

}
void BigData::ToString()
{
	INT64 temp=_value;//1111111111111111
	if (_value > 0)
		_StrData.append(1, '+');
	else
	{
		
		_StrData.append(1, '-');
	}
	if (_value < 0)
		temp = 0 - _value;
	while (temp)
	{
		char cStr = temp % 10 + '0';
		_StrData.append(1, cStr);
		temp /= 10;
	}
	int left = 1; int right = _StrData.size()-1;
	while (left < right)
	{
		swap(_StrData[left], _StrData[right]);
		++left;
		--right;

	}

}
bool BigData::IsLeftBig(const char *left, int lSize, const char *right, int rSize)
{
	if (lSize > rSize || lSize == rSize&&strcmp(left, right) >=0)
	{
		return true;
	}
	return false;
}

char BigData::SubLoop( char *left, int lSize, const char *right, int rSize)
{
	char Cret = '0';
	while (true)
	{
		if (!IsLeftBig(left, lSize, right, rSize))//如果左边小
		{
			break;
		}
		else//循环相减法
		{
			int lsize = lSize - 1;
			int rsize = rSize - 1;
			while (lsize>=0&&rsize>=0)
			{
				char ret = left[lsize] - '0';
				ret -= right[rsize] - '0';
				if (ret < 0)
				{
					left[lsize-1] -= 1;
					ret += 10;
				}
				left[lsize] = ret + '0';
				lsize--;
				rsize--;
			}
			while (*left == '0'&&lSize>0)
			{
				left++;
				lSize--;
			}
			Cret++;
		}
	}
	return Cret;
}



BigData BigData::operator +(BigData & bigdata)
{
	if (IsOver64() && bigdata.IsOver64())//都不溢出
	{
		if (_StrData[0] != bigdata._StrData[0])
		{
			return BigData(_value + bigdata._value);
		}
		INT64 tmp = MIN - _value;
		if ((_value > 0 && (MAX - _value >= bigdata._value)) || (_value < 0 && (tmp <= bigdata._value)))
		{
			return BigData(_value + bigdata._value);
		}
	}
	//至少有一个溢出
	//结果溢出
	if (_StrData[0] == bigdata._StrData[0])
		return BigData(Add(_StrData, bigdata._StrData).c_str());
	else return BigData(Sub(_StrData, bigdata._StrData).c_str());//符号位相反 取反。


}
string BigData::Add(string left, string right)//
{

	int iLeft = left.size();
	int iRight = right.size();	
	string ret;
	
	ret.append(1, left[0]);
	if (left[0] != right[0])
	{
		if (left[0] == '-')
			ret[0] = '+';
		if (right[0] == '+')
			ret[0] = '-';


	}
	if (iLeft < iRight)
	{
		swap(left, right);
		swap(iLeft, iRight);
	}
	ret.resize(iLeft + 1);

	char step = 0;
	for (int index = 1; index < iLeft; ++index)
	{
		char Cstr = left[iLeft - index] - '0' + step;
		if (index < iRight)
		{
			Cstr += (right[iRight - index] - '0');
		}

		ret[iLeft - index + 1] = Cstr % 10 + '0';
		step = Cstr / 10;
	}
	ret[1] = step + '0';
	return ret;
}

BigData BigData::operator -(BigData & bigdata)
{
	if (IsOver64() && bigdata.IsOver64())//两个不溢出
	{
		if (_StrData[0] == bigdata._StrData[0])
		{
			return BigData(_value - bigdata._value);
		}
		else
		{//10 -1  11   -11 1  -12
			if ((_value>0 || (_value<0) && MIN + bigdata._value <= _value))
				return BigData(_value - bigdata._value);
		}
	}
	 if (_StrData[0] != bigdata._StrData[0])
		{
			//bigdata._StrData[0] = _StrData[0];
			return BigData(Add(_StrData, bigdata._StrData).c_str());
		}

		else
		{
			
			return BigData(Sub(_StrData, bigdata._StrData).c_str());//符号位相同
		}
	


}

string BigData::Sub(string left, string right)
{
	//if (left[0]==right[])
	int iLeft = left.size();
	int iRight = right.size();
	string ret;
	char symble = left[0];

	if (iLeft < iRight || (iLeft == iRight&&left<right))//左边的xiao
	{
		swap(left, right);
		swap(iLeft, iRight);
		if (symble == '+')
			symble = '-';
		else
		{
			symble = '+';

		}
	}
	ret.resize(iLeft);

	ret[0] = symble;
	char step = 0;
	for (int index = 1; index < iLeft; ++index)
	{
		char Cstr = left[iLeft - index] - '0';
		if (index < iRight)
		{
			Cstr -= (right[iRight - index] - '0');
		}
		if (Cstr < 0)
		{
			left[iLeft - index - 1] -= 1;
			Cstr += 10;



		}
		ret[iLeft - index] = Cstr + '0';

	}

	return ret;
}

BigData BigData::operator *(BigData & bigdata)
{
	//
	if (IsOver64() && bigdata.IsOver64())
	{
		if (_StrData[0] == bigdata._StrData[0])//-10   -2    5
		{
			if ((_value > 0 && MAX / _value >= bigdata._value) || (_value < 0 && (MAX / _value <= bigdata._value)))
				return BigData(_value*bigdata._value);
		}
		else
		{
			if ((_value > 0 && MIN / _value >= bigdata._value) || (_value < 0 && (MIN / _value >= bigdata._value)))
				return BigData(_value*bigdata._value);
		}
	}

	return BigData(Mul(_StrData, bigdata._StrData).c_str());
}

string BigData::Mul(string left, string right)
{
	char symbol = '+';
	if (left[0] != right[0])
		 symbol = '-';
	int iLeft = left.size();
	int iRight = right.size();
	if (iLeft > iRight)
	{
		swap(left, right);
		swap(iLeft, iRight);
	}
	string ret;
	//ret.resize(iLeft + iRight - 1);
	ret.assign(iLeft + iRight -1, '0');
	ret[0] = symbol;
	int ioffset = 0;
	int rlen = ret.size();
	for (int index = 1; index < iLeft; ++index)
	{
		char Cleft = left[iLeft - index] - '0';
		char step = 0;
		if (Cleft == '0')
		{
			++ioffset;
			continue;
		}
		for (int index = 1; index < iRight; ++index)
		{
			char Cstr = Cleft*(right[iRight - index] - '0') + step;//取到字符
			Cstr += (ret[rlen - index - ioffset] - '0');//累加
			ret[rlen - index - ioffset] = (((Cstr) % 10)+'0' );
			step = Cstr / 10;
		}
		ret[rlen - iRight - ioffset] += step;
		ioffset++;
	}
	return ret;
}
BigData BigData::operator /(BigData & bigdata)
{
	assert(bigdata._value != 0);
	if (IsOver64() && bigdata.IsOver64())//不溢出
	{
		return BigData(_value / bigdata._value);
	}
	if (_StrData.size() < bigdata._StrData.size() || (_StrData.size() == bigdata._StrData.size()&&strcmp(_StrData.c_str() + 1, bigdata._StrData.c_str() + 1) < 0))
	{
		return BigData(INT64(0));
	}
	if (bigdata._StrData == "+1" || bigdata._StrData == "-1")
	{
		if (_StrData[0] != bigdata._StrData[0])
		{
			_StrData[0] = '-';
		}
		else _StrData[0] = '+';
		return *this;
	
	}
	if (strcmp(_StrData.c_str() + 1, bigdata._StrData.c_str() + 1) == 0)
	{
		if (_StrData[0] != bigdata._StrData[0])
		{
			return BigData(INT64(1));
		}
		else return BigData(INT64(-1));
	}
	return BigData(Div(_StrData, bigdata._StrData).c_str());
}
string BigData::Div(string left, string right)
{
	string ret;
	ret.append(1, '+');
	if (left[0] != right[0])
	{
		ret[0] = '-';
	}
	int LSize = left.size() - 1;
	int Rsize= right.size() - 1;
	char *Left = (char *)left.c_str() + 1;
	char *Right = (char *)right.c_str() + 1;
	int datalen = Rsize;
	for (int index = 0; index < LSize;)
	{
		if (!IsLeftBig(Left, datalen, Right, Rsize))//左边小于右边
		{
			ret.append(1, '0');
			datalen++;
			if (datalen + index>LSize)
				break;
		}
		else
		{
			ret.append(1, SubLoop(Left, datalen, Right, Rsize));
			while (*Left == '0'&&datalen > 0)
			{
				Left++;
				index++;
				datalen--;
			}
			datalen++;
			if (index + datalen > LSize)
				break;

		}
	}
	return ret;

}

