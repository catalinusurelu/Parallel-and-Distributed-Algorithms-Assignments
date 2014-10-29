#include<iostream>
using namespace std;

class Complex 
{
public:
   double x;
   double y;

	Complex()
	{
    	this->x = 0;
    	this->y = 0;
	}
    
    Complex(double x, double y)
    {
    	this->x = x;
    	this->y = y;
    }
    
 	Complex& operator=(const Complex& c)
    {
    	x = c.x;
		y = c.y;
		return *this;
    }
	
    Complex operator*(const Complex& c)
    {
    	Complex tmp;
    	tmp.x = x * c.x - y * c.y;
    	tmp.y = x * c.y + y * c.x;
    	return tmp;
    }
    Complex operator+(const Complex& c)
	 {
    	Complex tmp;
		tmp.x = x + c.x;
		tmp.y = y + c.y;
		return tmp;
	 }
    
    friend ifstream& operator>>(ifstream &in, Complex &c);
};

istream& operator>>(istream &in, Complex &c)
{
	in >> c.x >> c.y;
	return in;
}
