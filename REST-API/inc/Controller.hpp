#ifndef _CONTROLLER_H_
#define _CONTROLLER_H_

template <class T>
class Controller {
public:
    virtual ~Controller() = default;
    virtual T execute() const = 0;
};

#endif
