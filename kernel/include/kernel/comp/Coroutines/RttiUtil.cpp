#include <atomic>
#include <kernel/kernel.h>

// 类型识别工具
class RttiUil
{
    // 自增id
    template<typename T>
    class IncrementObj
    {
    public:
        static std::atomic<Int64> &GetMaxId()
        {
            static std::atomic<Int64> s_MaxId;    
            return s_MaxId;
        }
    };

    // 进程全局类型id生成器
    template<typename T>
    class GlobalTypeIdGenerator
    {
    public:
        static Int64 GenId()
        {
            static Int64 s_TypeId = RttiUil::GenId<RttiUil::CommonIncrementGenerator>();
            return s_TypeId;
        }
    };

    // 组件类型id生成器
    template<typename TOwner, typename TComp>
    class CompTypeIdGenerator
    {
    public:
        static Int64 GenId()
        {
            static Int64 s_TypeId = RttiUil::GenId<TOwner>();
            return s_TypeId;
        }
    };

public:
    // 产生进程全局唯一的id生成器
    class CommonIncrementGenerator{};

    // 生成进程全局唯一id
    template<typename TType>
    static Int64 GetObjTypeId()
    {
        return GlobalTypeIdGenerator<TType>::GenId();
    }

    // 生成TOwner作用于内的唯一id
    template<typename TOwner, typename TComp>
    static Int64 GetObjTypeId()
    {
        return CompTypeIdGenerator<TOwner, TComp>::GenId();
    }

    // id生成方法
    template<typename TGenerator>
    static Int64 GenId()
    {
        return ++IncrementObj<TGenerator>::GetMaxId();
    }

    // 获取TGenerator作用域范围内的当前最大id
    template<typename TGenerator>
    static Int64 GetCurMaxId()
    {
        return IncrementObj<TGenerator>::GetMaxId();
    }

    // id生成器基类
    class IGenerator
    {
    public:
        template<typename TComp>
        Int64 GenId()
        {
            static Int64 s_TypeId = GenFromSource();

            return s_TypeId;
        }

    protected:
        virtual Int64 GenFromSource() = 0;
    };


    // 服务的id生成器
    template<typename ServiceType>
    class IdGenerator : public IGenerator
    {
    protected:
        Int64 GenFromSource() override
        {
            return RttiUil::GenId<ServiceType>();
        }
    };

    // 兼容动态
    template<typename TComp>
    static Int64 GenFromSource(IGenerator *generator)
    {
        return generator->GenId<TComp>();
    }
};

// 组件基类
class LightComp
{
public:

};

// 具体的组件
class TestComp : public LightComp
{

};

// 对象
class LightObj
{

public:
    // 获取组件
    template<typename TComp>
    LightComp *GetComp()
    {
        return _comps[RttiUil::GetObjTypeId<LightObj, TComp>()];
    }

private:
    std::vector<LightComp *> _comps;
};