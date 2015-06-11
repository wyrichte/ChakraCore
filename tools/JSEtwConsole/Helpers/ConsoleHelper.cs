// <copyright file="ConsoleHelper.cs" company="Microsoft">
// Copyright (c) 2013 All Rights Reserved
// </copyright>
// <author>fcantonn</author>

namespace JsEtwConsole
{
    using System;

    static public class ConsoleHelper
    {
        /// <summary>
        /// Write an error message to console output
        /// </summary>
        /// <param name="format">format of the message to be outputted</param>
        /// <param name="args">optional arguments</param>
        public static void WriteError(string format, params object[] args)
        {
            Write(ConsoleColor.Red, true, "(E): " + format, args);
        }

        /// <summary>
        /// Write a warning message to console output
        /// </summary>
        /// <param name="format">format of the message to be outputted</param>
        /// <param name="args">optional arguments</param>
        public static void WriteWarning(string format, params object[] args)
        {
            Write(ConsoleColor.Yellow, true, "(W): " + format, args);
        }

        /// <summary>
        /// Write an informational message to console output
        /// </summary>
        /// <param name="format">format of the message to be outputted</param>
        /// <param name="args">optional arguments</param>
        public static void WriteInformation(string format, params object[] args)
        {
            Write(ConsoleColor.White, true, "(I): " + format, args);
        }

        /// <summary>
        /// Write a debug message to console output
        /// </summary>
        /// <param name="format">format of the message to be outputted</param>
        /// <param name="args">optional arguments</param>
        public static void WriteDebug(string format, params object[] args)
        {
            Write(ConsoleColor.DarkGray, true, "(D): " + format, args);
        }

        /// <summary>
        /// Write any other type of message to console output
        /// </summary>
        /// <param name="format">format of the message to be outputted</param>
        /// <param name="args">optional arguments</param>
        public static void WriteOther(string format, params object[] args)
        {
            Write(ConsoleColor.Gray, true, "     " + format, args);
        }

        /// <summary>
        /// Write a message to console output
        /// </summary>
        /// <param name="color">color to be used</param>
        /// <param name="newLine">indicates whether a new line is to be appended</param>
        /// <param name="format">format of the message to be outputted</param>
        /// <param name="args">optional arguments</param>
        public static void Write(ConsoleColor color, bool newLine, string format, params object[] args)
        {
            ConsoleColor currentColor = Console.ForegroundColor;

            try
            {
                Console.ForegroundColor = color;
                if (newLine)
                {
                    if (args == null)
                    {
                        Console.WriteLine(format);
                    }
                    else
                    {
                        Console.WriteLine(format, args);
                    }
                }
                else
                {
                    if (args == null)
                    {
                        Console.Write(format);
                    }
                    else
                    {
                        Console.Write(format, args);
                    }
                }
            }
            finally
            {
                Console.ForegroundColor = currentColor;
            }
        }
    }
}
