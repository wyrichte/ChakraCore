using System;
using System.Runtime.InteropServices;

namespace Microsoft.BPT.Tests.DirectAuthor
{
    public class HighResolutionTimer
    {
        private bool isPerfCounterSupported = false;
        private Int64 frequency = 0;
        private Int64 startTime = 0;
        private Int64 endTime = 0;

        // Windows CE native library with QueryPerformanceCounter().
        private const string lib = "coredll.dll";
        [DllImport("kernel32.dll", SetLastError = true)]
        private static extern int QueryPerformanceCounter(ref Int64 count);
        [DllImport("kernel32.dll", SetLastError = true)]
        private static extern int QueryPerformanceFrequency(ref Int64 frequency);

        public HighResolutionTimer()
        {
            // Query the high-resolution timer only if it is supported.
            // A returned frequency of 1000 typically indicates that it is not
            // supported and is emulated by the OS using the same value that is
            // returned by Environment.TickCount.
            // A return value of 0 indicates that the performance counter is
            // not supported.
            int returnVal = QueryPerformanceFrequency(ref this.frequency);

            if (returnVal != 0 && this.frequency != 1000)
            {
                // The performance counter is supported.
                this.isPerfCounterSupported = true;
            }
            else
            {
                // The performance counter is not supported. Use
                // Environment.TickCount instead.
                this.frequency = 1000;
            }
        }

        public Int64 Frequency
        {
            get
            {
                return this.frequency;
            }
        }

        public Int64 Ticks
        {
            get
            {
                Int64 tickCount = 0;

                if (isPerfCounterSupported)
                {
                    // Get the value here if the counter is supported.
                    QueryPerformanceCounter(ref tickCount);
                    return tickCount;
                }
                else
                {
                    // Otherwise, use Environment.TickCount.
                    return (Int64)Environment.TickCount;
                }
            }
        }

        public Int64 ElapsedTicks
        {
            get
            {
                Int64 elapsedTime = this.endTime - this.startTime;
                return elapsedTime > 0 ? elapsedTime : 0;
            }
        }

        public Int64 ElapseTimeInMilliseconds
        {
            get
            {
                return (Int64)(Math.Ceiling((this.ElapsedTicks * 1000.00) / this.Frequency));
            }
        }

        public void Start()
        {
            this.startTime = this.Ticks;
        }

        public void End()
        {
            this.endTime = this.Ticks;
        }
    }
}
