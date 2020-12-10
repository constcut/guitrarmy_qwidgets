/****************************************************************************
** Meta object code from reading C++ file 'athread.h'
**
** Created by: The Qt Meta Object Compiler version 67 (Qt 5.5.1)
**
** WARNING! All changes made in this file will be lost!
*****************************************************************************/

#include "athread.h"
#include <QtCore/qbytearray.h>
#include <QtCore/qmetatype.h>
#if !defined(Q_MOC_OUTPUT_REVISION)
#error "The header file 'athread.h' doesn't include <QObject>."
#elif Q_MOC_OUTPUT_REVISION != 67
#error "This file was generated using the moc from 5.5.1. It"
#error "cannot be used with the include files from this version of Qt."
#error "(The moc has changed too much.)"
#endif

QT_BEGIN_MOC_NAMESPACE
struct qt_meta_stringdata_ThreadLocal_t {
    QByteArrayData data[4];
    char stringdata0[34];
};
#define QT_MOC_LITERAL(idx, ofs, len) \
    Q_STATIC_BYTE_ARRAY_DATA_HEADER_INITIALIZER_WITH_OFFSET(len, \
    qptrdiff(offsetof(qt_meta_stringdata_ThreadLocal_t, stringdata0) + ofs \
        - idx * sizeof(QByteArrayData)) \
    )
static const qt_meta_stringdata_ThreadLocal_t qt_meta_stringdata_ThreadLocal = {
    {
QT_MOC_LITERAL(0, 0, 11), // "ThreadLocal"
QT_MOC_LITERAL(1, 12, 8), // "updateUI"
QT_MOC_LITERAL(2, 21, 0), // ""
QT_MOC_LITERAL(3, 22, 11) // "nowFinished"

    },
    "ThreadLocal\0updateUI\0\0nowFinished"
};
#undef QT_MOC_LITERAL

static const uint qt_meta_data_ThreadLocal[] = {

 // content:
       7,       // revision
       0,       // classname
       0,    0, // classinfo
       2,   14, // methods
       0,    0, // properties
       0,    0, // enums/sets
       0,    0, // constructors
       0,       // flags
       2,       // signalCount

 // signals: name, argc, parameters, tag, flags
       1,    0,   24,    2, 0x06 /* Public */,
       3,    0,   25,    2, 0x06 /* Public */,

 // signals: parameters
    QMetaType::Void,
    QMetaType::Void,

       0        // eod
};

void ThreadLocal::qt_static_metacall(QObject *_o, QMetaObject::Call _c, int _id, void **_a)
{
    if (_c == QMetaObject::InvokeMetaMethod) {
        ThreadLocal *_t = static_cast<ThreadLocal *>(_o);
        Q_UNUSED(_t)
        switch (_id) {
        case 0: _t->updateUI(); break;
        case 1: _t->nowFinished(); break;
        default: ;
        }
    } else if (_c == QMetaObject::IndexOfMethod) {
        int *result = reinterpret_cast<int *>(_a[0]);
        void **func = reinterpret_cast<void **>(_a[1]);
        {
            typedef void (ThreadLocal::*_t)();
            if (*reinterpret_cast<_t *>(func) == static_cast<_t>(&ThreadLocal::updateUI)) {
                *result = 0;
            }
        }
        {
            typedef void (ThreadLocal::*_t)();
            if (*reinterpret_cast<_t *>(func) == static_cast<_t>(&ThreadLocal::nowFinished)) {
                *result = 1;
            }
        }
    }
    Q_UNUSED(_a);
}

const QMetaObject ThreadLocal::staticMetaObject = {
    { &QThread::staticMetaObject, qt_meta_stringdata_ThreadLocal.data,
      qt_meta_data_ThreadLocal,  qt_static_metacall, Q_NULLPTR, Q_NULLPTR}
};


const QMetaObject *ThreadLocal::metaObject() const
{
    return QObject::d_ptr->metaObject ? QObject::d_ptr->dynamicMetaObject() : &staticMetaObject;
}

void *ThreadLocal::qt_metacast(const char *_clname)
{
    if (!_clname) return Q_NULLPTR;
    if (!strcmp(_clname, qt_meta_stringdata_ThreadLocal.stringdata0))
        return static_cast<void*>(const_cast< ThreadLocal*>(this));
    if (!strcmp(_clname, "PlayAnimationThr"))
        return static_cast< PlayAnimationThr*>(const_cast< ThreadLocal*>(this));
    return QThread::qt_metacast(_clname);
}

int ThreadLocal::qt_metacall(QMetaObject::Call _c, int _id, void **_a)
{
    _id = QThread::qt_metacall(_c, _id, _a);
    if (_id < 0)
        return _id;
    if (_c == QMetaObject::InvokeMetaMethod) {
        if (_id < 2)
            qt_static_metacall(this, _c, _id, _a);
        _id -= 2;
    } else if (_c == QMetaObject::RegisterMethodArgumentMetaType) {
        if (_id < 2)
            *reinterpret_cast<int*>(_a[0]) = -1;
        _id -= 2;
    }
    return _id;
}

// SIGNAL 0
void ThreadLocal::updateUI()
{
    QMetaObject::activate(this, &staticMetaObject, 0, Q_NULLPTR);
}

// SIGNAL 1
void ThreadLocal::nowFinished()
{
    QMetaObject::activate(this, &staticMetaObject, 1, Q_NULLPTR);
}
struct qt_meta_stringdata_ThreadWave_t {
    QByteArrayData data[4];
    char stringdata0[33];
};
#define QT_MOC_LITERAL(idx, ofs, len) \
    Q_STATIC_BYTE_ARRAY_DATA_HEADER_INITIALIZER_WITH_OFFSET(len, \
    qptrdiff(offsetof(qt_meta_stringdata_ThreadWave_t, stringdata0) + ofs \
        - idx * sizeof(QByteArrayData)) \
    )
static const qt_meta_stringdata_ThreadWave_t qt_meta_stringdata_ThreadWave = {
    {
QT_MOC_LITERAL(0, 0, 10), // "ThreadWave"
QT_MOC_LITERAL(1, 11, 8), // "updateUI"
QT_MOC_LITERAL(2, 20, 0), // ""
QT_MOC_LITERAL(3, 21, 11) // "nowFinished"

    },
    "ThreadWave\0updateUI\0\0nowFinished"
};
#undef QT_MOC_LITERAL

static const uint qt_meta_data_ThreadWave[] = {

 // content:
       7,       // revision
       0,       // classname
       0,    0, // classinfo
       2,   14, // methods
       0,    0, // properties
       0,    0, // enums/sets
       0,    0, // constructors
       0,       // flags
       2,       // signalCount

 // signals: name, argc, parameters, tag, flags
       1,    0,   24,    2, 0x06 /* Public */,
       3,    0,   25,    2, 0x06 /* Public */,

 // signals: parameters
    QMetaType::Void,
    QMetaType::Void,

       0        // eod
};

void ThreadWave::qt_static_metacall(QObject *_o, QMetaObject::Call _c, int _id, void **_a)
{
    if (_c == QMetaObject::InvokeMetaMethod) {
        ThreadWave *_t = static_cast<ThreadWave *>(_o);
        Q_UNUSED(_t)
        switch (_id) {
        case 0: _t->updateUI(); break;
        case 1: _t->nowFinished(); break;
        default: ;
        }
    } else if (_c == QMetaObject::IndexOfMethod) {
        int *result = reinterpret_cast<int *>(_a[0]);
        void **func = reinterpret_cast<void **>(_a[1]);
        {
            typedef void (ThreadWave::*_t)();
            if (*reinterpret_cast<_t *>(func) == static_cast<_t>(&ThreadWave::updateUI)) {
                *result = 0;
            }
        }
        {
            typedef void (ThreadWave::*_t)();
            if (*reinterpret_cast<_t *>(func) == static_cast<_t>(&ThreadWave::nowFinished)) {
                *result = 1;
            }
        }
    }
    Q_UNUSED(_a);
}

const QMetaObject ThreadWave::staticMetaObject = {
    { &QThread::staticMetaObject, qt_meta_stringdata_ThreadWave.data,
      qt_meta_data_ThreadWave,  qt_static_metacall, Q_NULLPTR, Q_NULLPTR}
};


const QMetaObject *ThreadWave::metaObject() const
{
    return QObject::d_ptr->metaObject ? QObject::d_ptr->dynamicMetaObject() : &staticMetaObject;
}

void *ThreadWave::qt_metacast(const char *_clname)
{
    if (!_clname) return Q_NULLPTR;
    if (!strcmp(_clname, qt_meta_stringdata_ThreadWave.stringdata0))
        return static_cast<void*>(const_cast< ThreadWave*>(this));
    if (!strcmp(_clname, "WaveMoveThr"))
        return static_cast< WaveMoveThr*>(const_cast< ThreadWave*>(this));
    return QThread::qt_metacast(_clname);
}

int ThreadWave::qt_metacall(QMetaObject::Call _c, int _id, void **_a)
{
    _id = QThread::qt_metacall(_c, _id, _a);
    if (_id < 0)
        return _id;
    if (_c == QMetaObject::InvokeMetaMethod) {
        if (_id < 2)
            qt_static_metacall(this, _c, _id, _a);
        _id -= 2;
    } else if (_c == QMetaObject::RegisterMethodArgumentMetaType) {
        if (_id < 2)
            *reinterpret_cast<int*>(_a[0]) = -1;
        _id -= 2;
    }
    return _id;
}

// SIGNAL 0
void ThreadWave::updateUI()
{
    QMetaObject::activate(this, &staticMetaObject, 0, Q_NULLPTR);
}

// SIGNAL 1
void ThreadWave::nowFinished()
{
    QMetaObject::activate(this, &staticMetaObject, 1, Q_NULLPTR);
}
QT_END_MOC_NAMESPACE
