#pragma once

// 定长内存池
#include<iostream>

// 不用using namespace std 以防污染命名
using std::cout;
using std::endl;

template<class T>
class ObjectPool {
	char* _memory = nullptr; //指向内存块指针，大块内存
	void* _freeList = nullptr; // 用来连接归还的空闲空间
	size_t _remanentBytes = 0; //记录大块内存在切分中剩余字节数

public:
	T* New() { // 申请T类型的空间
		T* obj = nullptr; // 最终返回的空间

		if (_freeList) {
			// _freeList不为空时，表示有回收的T大小的小块可以重复利用
			void* next = *(void**)_freeList;
			obj = (T*)_freeList;
			_freeList = next;
			// 头删操作
		}
		else { // 自由链表中没有块，也就没有可以重复利用的空间
				// _memory中剩余空间小于T的大小时候再开空间
			if (_remanentBytes < sizeof(T)) {  // 这样也会包含剩余空间为0的情况
				// 开128k的空间
				_remanentBytes = 128 * 1024;  
				_memory = (char*)malloc(128 * 1024);
				if (_memory == nullptr) {
					throw std::bad_alloc(); // 开失败了抛异常
				}
			}
			obj = (T*)_memory; // 给定一个T类型的大小
			// 判断一下T的大小，小于指针就给一个指针大小，大于指针就给T的大小
			size_t objSize = sizeof(T) < sizeof(void*) ? sizeof(void*) : sizeof(T);
			_memory += sizeof(T); // _memory后移一个T类型大小
			_remanentBytes -= sizeof(T); // 给出空间后_remanentBytes减少了T类型的大小
		}

		new(obj)T; // 通过定位new调用构造函数进行初始化
		return obj;
	}

	void Delete(T* obj) { // 回收还回来的小空间
		// 调用析构函数进行清理工作
		obj->~T();
		// 头插
		*(void**)obj = _freeList; // 新块指向旧块（或空）
		_freeList = obj; // 头指针指向新块
	}
};