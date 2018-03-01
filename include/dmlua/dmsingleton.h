
#ifndef __DMSINGLETON_H_INCLUDE__
#define __DMSINGLETON_H_INCLUDE__

template<typename T>
class TSingleton
{
private:
    typedef T  SingletonObj;
public:
    static SingletonObj* Instance(){ static SingletonObj s_oT; return &s_oT; }
};

template<typename T>
class TSafeSingleton
{
private:
    typedef T  SingletonObj;
public:
    class  TSafeCreator
    {
    public:
        TSafeCreator(){ TSafeSingleton<SingletonObj>::Instance();}
        inline void Do(){}
    };

    static TSafeCreator s_oCreator;
public:
    static SingletonObj* Instance(){ static SingletonObj s_oT; s_oCreator.Do(); return &s_oT; }

};

template <class T> typename
TSafeSingleton<T>::TSafeCreator TSafeSingleton<T>::s_oCreator;

#endif // __DMSINGLETON_H_INCLUDE__
