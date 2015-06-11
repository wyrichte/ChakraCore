//js repro code: new Animals.Bug803209.Class1().method1(0);
namespace Animals.Bug803209
{    
    class Class2
    {
        System.Enum e2;
        void foo(){
          var x=e2;
        }
    }
    public enum E1 { a, b, c } // must put after System.Enum 
    public sealed class Class1
    {
        public void Method1(E1 e)
        {
        }
    } 
}
