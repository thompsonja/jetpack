#ifndef OBSERVER_H
#define OBSERVER_H

#include <vector>

template<class T> class IObserver
{
public:
  IObserver(){}
  virtual ~IObserver(){}
  virtual void OnNotify(const T *subject, void *param) = 0;
};

template<class T> class ISubject
{
public:
  ISubject();
  virtual ~ISubject(){}
  void AddObserver(IObserver<T> *observer)
  {
    observers.push_back(observer);
  }
  void NotifyListeners(const T *msg, void *param = NULL)
  {
    for(unsigned int i = 0; i < observers.size(); i++)
    {
      OnNotify(msg, param)
    }
  }
private:
  std::vector<IObserver<T>*> observers;
};

#endif