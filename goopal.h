#ifndef GOOPAL_H
#define GOOPAL_H

#include <QObject>

class QFrame;

class Goopal : public QObject
{
    Q_OBJECT

public:
    ~Goopal();
    static Goopal* getInstance();

	QFrame* mainFrame = NULL; // 指向主窗口的指针

private:

    Goopal();
    static Goopal* goo;

    class CGarbo // 它的唯一工作就是在析构函数中删除CSingleton的实例
    {
    public:
        ~CGarbo()
        {
            if (Goopal::goo)
                delete Goopal::goo;
        }
    };
    static CGarbo Garbo; // 定义一个静态成员，在程序结束时，系统会调用它的析构函数
};
#endif // GOOPAL_H
