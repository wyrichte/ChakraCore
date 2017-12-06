//-------------------------------------------------------------------------------------------------------
// Copyright (C) Microsoft. All rights reserved.
// Licensed under the MIT license. See LICENSE.txt file in the project root for full license information.
//-------------------------------------------------------------------------------------------------------

// Verify that jshost doesn't leak memory when there is deferred work and the main execution throws.
Promise.resolve().then(function () { });
hi