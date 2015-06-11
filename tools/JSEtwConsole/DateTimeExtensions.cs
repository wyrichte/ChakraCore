// <copyright file="DateTimeExtensions.cs" company="Microsoft">
// Copyright (c) 2013 All Rights Reserved
// </copyright>
// <author>fcantonn</author>

namespace JsEtwConsole
{
    using System;
    using System.Globalization;
    using System.Threading;

    /// <summary>
    /// Represents extension methods for the DateTime structure.
    /// </summary>
    public static class DateTimeExtensions
    {
        #region Fields
        /// <summary>
        /// The local cache string that contains cached values of the year, month, day, hour, and minute values for the round trip ToString() method.
        /// </summary>
        private static string localRoundTripCacheString = null;

        /// <summary>
        /// The cached round trip ticks value.
        /// </summary>
        private static long localRoundTripCacheTicks = 0;

        /// <summary>
        /// The tick operator value to modulate by when determining changes for cache updates.
        /// (watch for minute interval changes).
        /// </summary>
        private static long localRoundTripTickModulateValue = 10000 * 1000 * 60;

        /// <summary>
        /// The lock object used to synchronize the cache reads/writes.
        /// </summary>
        private static ReaderWriterLockSlim localRoundTripLock = new ReaderWriterLockSlim();

        /// <summary>
        /// Time to wait for grabbing a write lock before aborting the write.
        /// </summary>
        private static int localRoundTripLockTimeoutInMilliseconds = 100;

        /// <summary>
        /// Stores the local hour offset for the current time zone to UTC time.
        /// </summary>
        private static string localUtcHourOffset = null;

        /// <summary>
        /// Stores the local minute offset for the current time zone to UTC time.
        /// </summary>
        private static string localUtcMinuteOffset = null;
        #endregion // Fields

        #region Lifetime Methods
        /// <summary>
        /// Initializes static members of the <see cref="DateTimeExtensions"/> class.
        /// </summary>
        static DateTimeExtensions()
        {
            DateTime local = DateTime.Now;
            DateTime utc = local.ToUniversalTime();

            DateTimeExtensions.localUtcHourOffset = (((utc.Hour - local.Hour) + 24) % 24).ToString("D2", CultureInfo.InvariantCulture);
            DateTimeExtensions.localUtcMinuteOffset = (((utc.Minute - local.Minute) + 60) % 60).ToString("D2", CultureInfo.InvariantCulture);
        }
        #endregion // Lifetime Methods

        #region Events
        #endregion // Events

        #region Properties
        #endregion // Properties

        #region Methods

        #region Public Methods
        /// <summary>
        /// Converts the <see cref="DateTime"/> object to a round-trip formatted string
        /// (similar to DateTime.ToString("o")).  Note that this method only works for times
        /// captured in the local timezone (or UTC timezone).
        /// </summary>
        /// <param name="dateTime">The <see cref="DateTime"/> object to convert.</param>
        /// <returns>The <see cref="DateTime"/> object in round-trip format.</returns>
        public static string ToCachedRoundTripLocalString(this DateTime dateTime)
        {
            bool shouldUpdateCache = false;
            long ticks = dateTime.Ticks;
            string roundTripString = null;

            if (DateTimeExtensions.localRoundTripLock.TryEnterReadLock(DateTimeExtensions.localRoundTripLockTimeoutInMilliseconds))
            {
                try
                {
                    long modulatedTicks = DateTimeExtensions.GetTicksModulatedValue(ticks);
                    shouldUpdateCache = modulatedTicks != DateTimeExtensions.localRoundTripCacheTicks;
                    roundTripString = DateTimeExtensions.localRoundTripCacheString;
                }
                finally
                {
                    DateTimeExtensions.localRoundTripLock.ExitReadLock();
                }
            }
            else
            {
                throw new TimeoutException("ReadLockTimeoutException");
            }

            if (shouldUpdateCache)
            {
                if (DateTimeExtensions.localRoundTripLock.TryEnterWriteLock(DateTimeExtensions.localRoundTripLockTimeoutInMilliseconds))
                {
                    try
                    {
                        // Check again to make sure another thread didn't already update.
                        long modulatedTicks = DateTimeExtensions.GetTicksModulatedValue(ticks);
                        shouldUpdateCache = modulatedTicks != DateTimeExtensions.localRoundTripCacheTicks;

                        if (shouldUpdateCache)
                        {
                            DateTimeExtensions.localRoundTripCacheString = String.Join(
                                String.Empty,
                                new string[] 
                                { 
                                    dateTime.Year.ToString("D4", CultureInfo.InvariantCulture), 
                                    "-", 
                                    dateTime.Month.ToString("D2", CultureInfo.InvariantCulture), 
                                    "-", 
                                    dateTime.Day.ToString("D2", CultureInfo.InvariantCulture), 
                                    "T", 
                                    dateTime.Hour.ToString("D2", CultureInfo.InvariantCulture), 
                                    ":", 
                                    dateTime.Minute.ToString("D2", CultureInfo.InvariantCulture), 
                                    ":" 
                                });

                            DateTimeExtensions.localRoundTripCacheTicks = modulatedTicks;

                            roundTripString = DateTimeExtensions.localRoundTripCacheString;
                        }
                    }
                    finally
                    {
                        DateTimeExtensions.localRoundTripLock.ExitWriteLock();
                    }
                }
                else
                {
                    throw new TimeoutException("WriteLockTimeoutException");
                }
            }

            long tenMillionthsOfASecondValue = dateTime.Ticks % 10000000;

            roundTripString += String.Join(
                String.Empty,
                new string[] { dateTime.Second.ToString("D2", CultureInfo.InvariantCulture), ".", tenMillionthsOfASecondValue.ToString("D7", CultureInfo.InvariantCulture) });

            if (dateTime.Kind == DateTimeKind.Local)
            {
                roundTripString += String.Join(
                    String.Empty,
                    new string[] { "-", DateTimeExtensions.localUtcHourOffset, ":", DateTimeExtensions.localUtcMinuteOffset });
            }
            else if (dateTime.Kind == DateTimeKind.Utc)
            {
                roundTripString += "Z";
            }

            return roundTripString;
        }
        #endregion // Public Methods

        #region Private Methods
        /// <summary>
        /// Gets the ticks modulated value which is the value that must change in order for a cache refresh to occur.
        /// </summary>
        /// <param name="ticks">The number of ticks to modulate.</param>
        /// <returns>The ticks modulated value which is the value that must change in order for a cache refresh to occur.</returns>
        private static long GetTicksModulatedValue(long ticks)
        {
            return ticks / DateTimeExtensions.localRoundTripTickModulateValue;
        }
        #endregion // Private Methods

        #endregion // Methods
    }
}
