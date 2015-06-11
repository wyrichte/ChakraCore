//-----------------------------------------------------------------------
// <copyright file="Logger.cs" company="Microsoft">
//     Copyright (C) Microsoft 2013, All rights reserved.
// </copyright>
//-----------------------------------------------------------------------

namespace ChakraUnitTestTelemetry
{
    using System;
    using System.Collections.Concurrent;
    using System.Collections.Generic;
    using System.Diagnostics.Contracts;
    using System.Text;
    using System.Threading;

    /// <summary>
    /// Represents helper methods for outputting information to the console
    /// and for tracing.
    /// </summary>
    public class Logger
    {
        /// <summary>
        /// Lock object used to synchronize writing to the console with
        /// specific foreground colors.
        /// </summary>
        private static object consoleWriteLock = new object();

        /// <summary>
        /// The thread-safe queue that stores all traced messages in the application.
        /// </summary>
        private static ConcurrentQueue<string> traceQueue = new ConcurrentQueue<string>();

        /// <summary>
        /// Thread safe dictionary that tracks the trace messages for specified threads.
        /// </summary>
        private static ConcurrentDictionary<int, ConcurrentQueue<string>> traceThreadQueues =
            new ConcurrentDictionary<int, ConcurrentQueue<string>>();

        /// <summary>
        /// Logs a verbose message to the output. This method is thread safe.
        /// </summary>
        /// <param name="message">The message to log.</param>
        /// <param name="args">The arguments to format into the message.</param>
        public static void Verbose(string message, params object[] args)
        {
            ValidateMessageAndArgsParameters(message, args);
            OutputToConsoleAndTrace("V", message, ConsoleColor.Gray, args);
        }

        /// <summary>
        /// Logs an informational message to the output. This method is thread safe.
        /// </summary>
        /// <param name="message">The message to log.</param>
        /// <param name="args">The arguments to format into the message.</param>
        public static void Info(string message, params object[] args)
        {
            ValidateMessageAndArgsParameters(message, args);
            OutputToConsoleAndTrace("I", message, ConsoleColor.White, args);
        }

        /// <summary>
        /// Logs a warning message to the output. This method is thread safe.
        /// </summary>
        /// <param name="message">The message to log.</param>
        /// <param name="args">The arguments to format into the message.</param>
        public static void Warning(string message, params object[] args)
        {
            ValidateMessageAndArgsParameters(message, args);
            OutputToConsoleAndTrace("W", message, ConsoleColor.Yellow, args);
        }

        /// <summary>
        /// Logs an error message to the output. This method is thread safe.
        /// </summary>
        /// <param name="message">The message to log.</param>
        /// <param name="args">The arguments to format into the message.</param>
        public static void Error(string message, params object[] args)
        {
            ValidateMessageAndArgsParameters(message, args);
            OutputToConsoleAndTrace("E", message, ConsoleColor.Red, args);
        }

        /// <summary>
        /// Returns all logs that have been stored in the trace queue as a string.
        /// Each log entry is new line delimited.
        /// </summary>
        /// <returns>A string that stores each trace log on a new line.</returns>
        public static string GetTraceLogs()
        {
            Contract.Ensures(Contract.Result<string>() != null);
            return string.Join(Environment.NewLine, traceQueue);
        }

        /// <summary>
        /// Gets the per thread trace logs.
        /// </summary>
        /// <returns>
        /// The trace logs dumped for each thread that ran during tracing.
        /// The key of each pair represents the thread index and the value is the log.
        /// </returns>
        public static IEnumerable<KeyValuePair<int, string>> GetPerThreadTraceLogs()
        {
            Contract.Ensures(Contract.Result<IEnumerable<KeyValuePair<int, string>>>() != null);

            foreach (KeyValuePair<int, ConcurrentQueue<string>> pair in traceThreadQueues)
            {
                int threadId = pair.Key;
                ConcurrentQueue<string> queue = pair.Value;

                yield return new KeyValuePair<int, string>(threadId, string.Join(Environment.NewLine, queue));
            }
        }

        /// <summary>
        /// Outputs to the console and traces the message in the internal trace queue.
        /// This method is thread safe.
        /// </summary>
        /// <param name="severityPrefix">The severity prefix to append to the message.</param>
        /// <param name="message">The message to trace.</param>
        /// <param name="color">The color to render the message to the console as (foreground color).</param>
        /// <param name="args">The arguments to format into the message.</param>
        private static void OutputToConsoleAndTrace(string severityPrefix, string message, ConsoleColor color, params object[] args)
        {
            Contract.Requires(!string.IsNullOrWhiteSpace(severityPrefix));
            ValidateMessageAndArgsParameters(message, args);
            Contract.Requires(Settings.Default.MaxLogQueueEntryCount > 0);

            // Get information about the thread we're currently running on.
            int threadId = Thread.CurrentThread.ManagedThreadId;

            string prefixedMessage = string.Format(message, args);
            prefixedMessage = string.Format(
                "{0:00000000}|{1}|{2}|{3}",
                threadId,
                DateTime.UtcNow.ToString("yyyy/MM/dd_HH:mm:ss.fff"),
                severityPrefix,
                prefixedMessage);

            lock (consoleWriteLock)
            {
                // Lock when changing the color so the right text is set.
                Console.ForegroundColor = color;
                Console.WriteLine(prefixedMessage);
            }

            // Remove any entries that go beyond the max allowed entry count.  Note this won't
            // be guaranteed to be exact due to the potential for multi-threading, but we just
            // need to hover around the range to keep allocations in check.
            while (traceQueue.Count >= Settings.Default.MaxLogQueueEntryCount)
            {
                string result;
                traceQueue.TryDequeue(out result);
            }

            // Add the new message to the trace queue.
            traceQueue.Enqueue(prefixedMessage);

            // Add the new message to the proper queue for the current thread.
            AddMessageToThreadQueue(threadId, prefixedMessage);
        }

        /// <summary>
        /// Adds the message to thread queue for the specified thread.
        /// </summary>
        /// <param name="threadId">The thread id of the thread to add to.</param>
        /// <param name="prefixedMessage">The prefixed trace message to add.</param>
        private static void AddMessageToThreadQueue(int threadId, string prefixedMessage)
        {
            Contract.Requires(!string.IsNullOrWhiteSpace(prefixedMessage));

            ConcurrentQueue<string> threadTraceQueue = null;
            if (!traceThreadQueues.ContainsKey(threadId))
            {
                traceThreadQueues.TryAdd(threadId, new ConcurrentQueue<string>());
            }

            Contract.Assert(traceThreadQueues.ContainsKey(threadId));

            threadTraceQueue = traceThreadQueues[threadId];
            Contract.Assert(threadTraceQueue != null);

            // Remove any entries that go beyond the max allowed entry count.  Note this won't
            // be guaranteed to be exact due to the potential for multi-threading, but we just
            // need to hover around the range to keep allocations in check.
            while (threadTraceQueue.Count >= Settings.Default.MaxLogQueueEntryCount)
            {
                string result;
                threadTraceQueue.TryDequeue(out result);
            }

            threadTraceQueue.Enqueue(prefixedMessage);
        }

        /// <summary>
        /// Validates the message and args parameters conform to the specified code contract.
        /// </summary>
        /// <param name="message">The message to validate.</param>
        /// <param name="args">The args to validate.</param>
        [ContractAbbreviator]
        private static void ValidateMessageAndArgsParameters(string message, params object[] args)
        {
            Contract.Requires(!string.IsNullOrWhiteSpace(message));
            Contract.Requires(args != null);
        }
    }
}
