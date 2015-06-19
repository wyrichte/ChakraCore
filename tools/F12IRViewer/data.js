//
// Copyright (C) Microsoft. All rights reserved.
//

var HeaderMessage = new Enum([
  'CONNECTION_SUCCESSFUL',  // notify the target that the host has made a successful connection
  'REQUEST_FUNCTION_LIST',  // request the target for the function list
  'REQUEST_JIT',            // request for the JIT from a given function
]);

var RemoteMessage = new Enum([
  'ACK_CONNECTION_SUCCESSFUL',
  'FUNCTION_LIST',          // message contains function list data
  'IR_VIEWER_JIT',          // message contains IRViewer JIT info
]);
