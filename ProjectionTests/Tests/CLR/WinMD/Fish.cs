using System;
using System.Collections.Generic;
using Windows.Foundation.Collections;
namespace Animals
{
    public interface ILikeToSwim
    {
        void SingTheSwimmingSong(out string lyrics);
    }
    public interface IFish : ILikeToSwim
    {
        void GetNumFins(out int numberOfFins);
        void SetNumFins(int numberOfFins);
        void MarshalIFish(IFish _in, out IFish _out);
        void MarshalILikeToSwim(ILikeToSwim _in, out ILikeToSwim _out);
        void MarshalIFishToFish(IFish _in, out Fish _outFish);
        void MarshalILikeToSwimToFish(ILikeToSwim _in, out ILikeToSwim _out);
        string Name { get; set; }
    }
    public interface IFastSigInterface
    {

        void GetOneVector(out IList<int> outVal);
        void GetNullAsVector(out IList<int> outVal);
        void GetOneObservableVector(out IObservableVector<int> outVal);
        void GetNullAsObservableVector(out IObservableVector<int> outVal);
        void GetOneAnimal(out IAnimal outVal);
        void GetNullAsAnimal(out IAnimal outVal);
        void GetOneMap(out IDictionary<string, int> outVal);
        void GetNullAsMap(out IDictionary<string, int> outVal);
        void GetOnePropertyValue(out object outVal);
        void GetNullAsPropertyValue(out object outVal);
        void GetOneEmptyGRCNInterface(out IEmptyGRCN outValue);
        void GetOneEmptyGRCNNull(out IEmptyGRCN outValue);
        void GetOneEmptyGRCNFail(out IEmptyGRCN outValue);

    }
    public sealed class Fish : IFish, IFastSigInterface
    {
        private int m_NumFins;
        private string m_name;
        public Fish()
        {
            m_NumFins = 5;
            m_name = string.Empty;
        }
        #region "IFish"
        public void GetNumFins(out int numberOfFins)
        {
            numberOfFins = m_NumFins;
        }
        public void SetNumFins(int numberOfFins)
        {
            m_NumFins = numberOfFins;
        }
        public void MarshalIFish(IFish _in, out IFish _out)
        {
            _out = _in;
        }
        public void MarshalIFishToFish(IFish _in, out Fish _outFish)
        {
            _outFish = (Fish)_in;
        }
        public void MarshalILikeToSwim(ILikeToSwim _in, out ILikeToSwim _out)
        {
            _out = _in;
        }
        public void MarshalILikeToSwimToFish(ILikeToSwim _in, out ILikeToSwim _out)
        {
            _out = _in;
        }
        public string Name
        {
            get
            {
                return m_name;
            }
            set
            {
                m_name = value;
            }
        }
        #endregion
        #region "ILikeToSwim"
        public void SingTheSwimmingSong(out string lyrics)
        {
            lyrics = "I feed from the bottom, you feed from the top \nI live upon morsels you happen to drop \nAnd coffee that somehow leaks out of your cup \nIf nothing comes down then I'm forced to swim up \n";
        }
        #endregion

        #region "IFastSigInterface"
        public void GetOneVector(out IList<int> outVal)
        {
            outVal = new List<int>();
            for (int i = 1; i < 4; i++)
            {
                outVal.Add(i);
            }
        }

        public void GetNullAsVector(out IList<int> outVal)
        {
            outVal = null;
        }

        public void GetOneObservableVector(out IObservableVector<int> outVal)
        {
            outVal = new MyObservableVector<int>();
            for (int i = 1; i < 5; i++)
                outVal.Add(i);
        }

        public void GetNullAsObservableVector(out IObservableVector<int> outVal)
        {
            outVal = null;
        }

        public void GetOneAnimal(out IAnimal outVal)
        {
            outVal = new Animal();
        }

        public void GetNullAsAnimal(out IAnimal outVal)
        {
            outVal = null;
        }

        public void GetOneMap(out IDictionary<string, int> outVal)
        {
            outVal = new Dictionary<string, int>();
            outVal.Add("by", 2);
            outVal.Add("Hundred",7);            
            outVal.Add("Hundred And Fifty",17);
        }

        public void GetNullAsMap(out IDictionary<string, int> outVal)
        {
            outVal = null;
        }

        public void GetOnePropertyValue(out object outVal)
        {
            double? d = 10.5;
            outVal = d;
        }

        public void GetNullAsPropertyValue(out object outVal)
        {
            outVal = null;
        }

        public void GetOneEmptyGRCNInterface(out IEmptyGRCN outValue)
        {
            outValue = new CEmptyGRCNInterface();
        }

        public void GetOneEmptyGRCNNull(out IEmptyGRCN outValue)
        {
            outValue = new CEmptyGRCN();
        }

        public void GetOneEmptyGRCNFail(out IEmptyGRCN outValue)
        {
            outValue = new CEmptyFailingGRCNString();
        }
        #endregion
    }
    //Try if you can remove IPuppy
    public interface IPuppy
    {
        int WagTail(int numberOfHeadPats);
    }
    public sealed class Pomapoodle : IPuppy
    {
        private Pomapoodle() { }
        public int WagTail(int numberOfHeadPats)
        {
            return 2 * numberOfHeadPats - 1;
        }
        public static void EatCookies(int numberOfCookies, out int cookiesEaten)
        {
            if (numberOfCookies <= 0)
                throw new ArgumentException();
            cookiesEaten = numberOfCookies - 1;
            if (CookiesEatenEvent != null)
                CookiesEatenEvent(null, cookiesEaten);
        }
        public static event CookiesEatenHandler CookiesEatenEvent;
    }
    public sealed class EmptyClass
    {
        private EmptyClass() { }
        public void dummyMethod() { }
    }
    //Try if you can remove IExtinct
    public sealed class DodoBird : IExtinct
    {
        public void HasTeeth(out bool res)
        {
            throw new NotImplementedException();
        }
        public void IsExtinct(out bool res)
        {
            res = true;
        }
    }
    public sealed class Turkey: Fabrikam.Kitchen.IBurgerMaster
    {
        public void GetNumFeathers(out int feathers)
        {
            feathers = 100;
        }
        public void ToSandwich(int baconSlices, out int hasMayo)
        {
            hasMayo = 0;
            if (baconSlices > 0)
            {
                hasMayo = 1;
            }
        }
        public void ToSandwich(out int hasMayo)
        {
            hasMayo = 1;
        }

        public int MakeBurger(uint baconSlices, uint cheeseSlices)
        {                       
            if (baconSlices>0 && cheeseSlices>0)  
            {
                return 0;
            }
            return 50;
        }

        public int MakeBurger()
        {
            return 100;
        }
    }
    public sealed class Elephant
    {
        private DateTimeOffset m_age;
        private TimeSpan m_timeToGetToSixtyMPH;
        private const Int64 ManagedUtcTicksAtNativeZero = 504911232000000000;
        public Elephant()
        {
            //string dateString = " 1/1/1601 00:00:00 AM";
            //DateTime date1 = DateTime.Parse(dateString,
            //                          System.Globalization.CultureInfo.InvariantCulture); 
            m_age = new DateTimeOffset(1601, 1, 1, 0, 0, 0, 0, TimeSpan.Zero);
            m_timeToGetToSixtyMPH = new TimeSpan(2000);
        }
        public DateTimeOffset GetAge()
        {
            return m_age;
        }
        public long GetAgeTicks()
        {
            Int64 dateTime = m_age.UtcTicks - ManagedUtcTicksAtNativeZero;
            return dateTime;
        }
        public void SetAge(DateTimeOffset age)
        {
            if (age == null)
                throw new ArgumentException();
            m_age = age;
        }
        public void SetAgeTicks(long age)
        {
            Int64 managedUtcTicks = ManagedUtcTicksAtNativeZero + age;
            DateTimeOffset managedUtcDTO = new DateTimeOffset(managedUtcTicks, TimeSpan.Zero);
            m_age = managedUtcDTO.ToLocalTime();
        }
        public TimeSpan GetTimeToGetToSixtyMPH()
        {
            return m_timeToGetToSixtyMPH;
        }
        public void SetTimeToGetToSixtyMPH(TimeSpan timespan)
        {
            m_timeToGetToSixtyMPH = timespan;
        }
        public void StartLifeNow()
        {
            m_age = DateTimeOffset.Now;
        }
    }
    public interface IDino
    {
        int Height { get; }
        void CanRoar(out bool result);
        void hasTeeth(out bool res);
        void Roar(int numtimes);
    }
    public interface IExtinct
    {
        void HasTeeth(out bool res);
        void IsExtinct(out bool res);
    }
    public sealed class Dino : IDino, IExtinct
    {
        private bool m_canRoar;
        public static event FossilsFoundHandler FossilsFoundEvent;
        public Dino()
        {
            m_canRoar = false;
        }
        void IExtinct.HasTeeth(out bool res)
        {
            res = false;
        }
        public void IsExtinct(out bool res)
        {
            res = true;
        }
        public void CanRoar(out bool result)
        {
            result = m_canRoar;
        }
        public int Height
        {
            get { return 5; }
        }
        public void Roar(int numtimes)
        {
            for (int i = 0; i < numtimes; i++)
            {
                //System.Console.Write("Roar!\n");
            }
        }
        void IDino.hasTeeth(out bool res)
        {
            res = true;
        }
        public static void LookForFossils(int timeSpent, out int fossilsFound)
        {
            fossilsFound = timeSpent / 4;
            if (FossilsFoundEvent != null)
            {
                FossilsFoundEvent(null, fossilsFound);
            }
        }
        public static void InspectDino(IDino specimen, out string results)
        {
            bool hasTeeth;
            int height = specimen.Height;
            specimen.hasTeeth(out hasTeeth);
            string hasTeethStr;
            if (hasTeeth)
            {
                hasTeethStr = "true";
            }
            else
            {
                hasTeethStr = "false";
            }
            results = "Height: " + height + "\nHasTeeth: " + hasTeethStr;
        }
        public static bool IsScary
        {
            get;
            set;
        }
    }
}
