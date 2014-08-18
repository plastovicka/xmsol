/*
 (C) 2003 Petr Lastovicka

 This program is free software; you can redistribute it and/or
 modify it under the terms of the GNU General Public License.
 */
#ifndef DARRAYH
#define DARRAYH

#pragma warning(disable:4710)

template <class T> class Darray
{
public:
	T *array;
	int len;
	int capacity;

	Darray(int n=16);
	~Darray(){ delete[] array; }
	T *operator++(int);
	T *operator--(int);
	T *operator+=(int d);
	T *operator-=(int d);
	operator T*(){ return array; }
	void setLen(int n);
	void setCapacity(int m);
	void reset(){ setLen(0); }
};

template <class T> Darray<T>::Darray(int n)
{
	len=0;
	array=0;
	capacity=-n;
}

template <class T> T *Darray<T>::operator+=(int d)
{
	setLen(len+d);
	return &array[len-d];
}

template <class T> T *Darray<T>::operator-=(int d)
{
	setLen(len-d);
	return &array[len];
}

template <class T> T *Darray<T>::operator++(int)
{
	setLen(len+1);
	return &array[len-1];
}

template <class T> T *Darray<T>::operator--(int)
{
	setLen(len-1);
	return &array[len];
}

template <class T> void Darray<T>::setLen(int n)
{
	if(n>capacity){
		if(capacity<0) setCapacity((-capacity<n) ? n : -capacity);
		else setCapacity(n+(n>>2));
	}
	len=n;
}

template <class T> void Darray<T>::setCapacity(int m)
{
	if(m!=capacity){
		if(m<len) m=len;
		T *a = array;
		array = (T*) operator new(m*sizeof(T));
		memcpy(array, a, len*sizeof(T));
		operator delete(a);
		capacity=m;
	}
}

template <class T> void deleteDarray(T &a)
{
	for(int i=0; i<a.len; i++){
		delete a[i];
	}
	a.len=0;
}

#endif

