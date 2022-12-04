// Author: EricYonng
// Date:2022-11-21
// Brief:

namespace ProtoPackage.Attributes;

// Proto消息
[AttributeUsage(AttributeTargets.Class)]
public class ProtoMessageAttribute : System.Attribute
{
    public int Opcode { get; init; }

    public ProtoMessageAttribute(int opcode)
    {
        
    }
}