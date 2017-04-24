#include "Vector.h"                      
inline float Vector3::length()   
{   
	return (float)( x * x + y * y + z * z );   
}   

Vector3 Vector3::normalize()   
{   
	float len = length();                 
	if( len == 0 )   
		len = 1;   
	x = x / len;   
	y = y / len;   
	z = z / len;   

	return *this;   
}   
  
float Vector3::dotProduct(const Vector3& v)   
{   
	return ( x * v.x + y * v.y + z * v.z );   
}   

Vector3 Vector3::crossProduct(const Vector3& v)   
{   
	Vector3 vec;   

	vec.x = y * v.z - z * v.y;   
	vec.y = z * v.x - x * v.z;   
	vec.z = x * v.y - y * v.x;   

	return vec;   
}   
 
Vector3 Vector3::operator +(const Vector3& v)   
{   
	Vector3 vec;   

	vec.x = x + v.x;   
	vec.y = y + v.y;   
	vec.z = z + v.z;   

	return vec;   
}   

Vector3 Vector3::operator -(const Vector3& v)   
{   
	Vector3 vec;   

	vec.x = x - v.x;   
	vec.y = y - v.y;   
	vec.z = z - v.z;   

	return vec;   
}   
 
Vector3 Vector3::operator *(float scale)   
{   
	x = x * scale;   
	y = y * scale;   
	z = z * scale;   

	return *this;   
}   
 
Vector3 Vector3::operator /(float scale)   
{   
	if(scale != 0.0)   
	{      
		x = x / scale;   
		y = y / scale;   
		z = z / scale;   
	}   
	return *this;   
}   

Vector3 Vector3::operator -()   
{   
	Vector3 vec( - x,- y, - z);   
	return vec;   
}   