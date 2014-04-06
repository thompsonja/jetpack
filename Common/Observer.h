#ifndef OBSERVER_H
#define OBSERVER_H

#include <algorithm>
#include <vector>
#include <memory>

template<class T> class IObserver
{
public:
  IObserver(){}
  virtual ~IObserver(){}
  virtual void OnNotify(const T &subject, void *param) = 0;
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

  void RemoveObserver(IObserver<T> *observer)
  {
    observers.erase(std::remove(observers.begin(), observers.end(), observer), observers.end());
  }

  void NotifyListeners(const T &msg, void *param = NULL)
  {
    for(auto& s : observers)
    {
      s->OnNotify(msg, param);
    }
  }
private:
  std::vector<std::shared_ptr<IObserver<T>>> observers;
};

#endif