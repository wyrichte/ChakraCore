WScript.Echo(Debug);

WScript.Echo(Debug.MS_ASYNC_OP_STATUS_SUCCESS);
WScript.Echo(Debug.MS_ASYNC_OP_STATUS_CANCELED);
WScript.Echo(Debug.MS_ASYNC_OP_STATUS_ERROR);
WScript.Echo(Debug.MS_ASYNC_CALLBACK_STATUS_ASSIGN_DELEGATE);
WScript.Echo(Debug.MS_ASYNC_CALLBACK_STATUS_JOIN);
WScript.Echo(Debug.MS_ASYNC_CALLBACK_STATUS_CHOOSEANY);
WScript.Echo(Debug.MS_ASYNC_CALLBACK_STATUS_CANCEL);
WScript.Echo(Debug.MS_ASYNC_CALLBACK_STATUS_ERROR);

WScript.Echo(Debug.msTraceAsyncOperationStarting);
WScript.Echo(Debug.msTraceAsyncCallbackStarting);
WScript.Echo(Debug.msTraceAsyncCallbackCompleted);
WScript.Echo(Debug.msUpdateAsyncCallbackRelation);
WScript.Echo(Debug.msTraceAsyncOperationCompleted);

WScript.Echo(JSON.stringify(Object.getOwnPropertyDescriptor(Debug,"MS_ASYNC_OP_STATUS_SUCCESS")));
WScript.Echo(JSON.stringify(Object.getOwnPropertyDescriptor(Debug,"MS_ASYNC_OP_STATUS_CANCELED")));
WScript.Echo(JSON.stringify(Object.getOwnPropertyDescriptor(Debug,"MS_ASYNC_OP_STATUS_ERROR")));
WScript.Echo(JSON.stringify(Object.getOwnPropertyDescriptor(Debug,"MS_ASYNC_CALLBACK_STATUS_ASSIGN_DELEGATE")));
WScript.Echo(JSON.stringify(Object.getOwnPropertyDescriptor(Debug,"MS_ASYNC_CALLBACK_STATUS_JOIN")));
WScript.Echo(JSON.stringify(Object.getOwnPropertyDescriptor(Debug,"MS_ASYNC_CALLBACK_STATUS_CHOOSEANY")));
WScript.Echo(JSON.stringify(Object.getOwnPropertyDescriptor(Debug,"MS_ASYNC_CALLBACK_STATUS_CANCEL")));
WScript.Echo(JSON.stringify(Object.getOwnPropertyDescriptor(Debug,"MS_ASYNC_CALLBACK_STATUS_ERROR")));

WScript.Echo(JSON.stringify(Object.getOwnPropertyDescriptor(Debug,"msTraceAsyncOperationStarting")));
WScript.Echo(JSON.stringify(Object.getOwnPropertyDescriptor(Debug,"msTraceAsyncCallbackStarting")));
WScript.Echo(JSON.stringify(Object.getOwnPropertyDescriptor(Debug,"msTraceAsyncCallbackCompleted")));
WScript.Echo(JSON.stringify(Object.getOwnPropertyDescriptor(Debug,"msUpdateAsyncCallbackRelation")));
WScript.Echo(JSON.stringify(Object.getOwnPropertyDescriptor(Debug,"msTraceAsyncOperationCompleted")));

var opId1 = Debug.msTraceAsyncOperationStarting("somestring", 1);
var opId2 = Debug.msTraceAsyncOperationStarting("someotherstring", 2);
var opId3 = Debug.msTraceAsyncOperationStarting(1, 3);
var opId4 = Debug.msTraceAsyncOperationStarting(undefined, -1);
var opId5 = Debug.msTraceAsyncOperationStarting();
var opId6 = Debug.msTraceAsyncOperationStarting(1);
var opId7 = Debug.msTraceAsyncOperationStarting(undefined);
var opId8 = Debug.msTraceAsyncOperationStarting(0);

WScript.Echo(opId1);
WScript.Echo(opId2);
WScript.Echo(opId3);
WScript.Echo(opId4);
WScript.Echo(opId5);
WScript.Echo(opId6);
WScript.Echo(opId7);
WScript.Echo(opId8);

WScript.Echo(Debug.msTraceAsyncOperationCompleted(opId1, 1, 1));
WScript.Echo(Debug.msTraceAsyncOperationCompleted(opId2));
WScript.Echo(Debug.msTraceAsyncOperationCompleted(opId3, 2));
WScript.Echo(Debug.msTraceAsyncOperationCompleted(opId4, 3, 2));
WScript.Echo(Debug.msTraceAsyncOperationCompleted(opId5, -1, -1));
WScript.Echo(Debug.msTraceAsyncOperationCompleted(-1));
WScript.Echo(Debug.msTraceAsyncOperationCompleted(-2));
WScript.Echo(Debug.msTraceAsyncOperationCompleted());
WScript.Echo(Debug.msTraceAsyncOperationCompleted(opId6, 0));
WScript.Echo(Debug.msTraceAsyncOperationCompleted(opId7));
WScript.Echo(Debug.msTraceAsyncOperationCompleted(opId8));

WScript.Echo(Debug.msTraceAsyncCallbackStarting(opId8, 1));
WScript.Echo(Debug.msTraceAsyncCallbackCompleted());

WScript.Echo(Debug.msTraceAsyncCallbackStarting(opId6, 2));
WScript.Echo(Debug.msTraceAsyncCallbackCompleted(0));

WScript.Echo(Debug.msTraceAsyncCallbackStarting(opId3, 0));
WScript.Echo(Debug.msTraceAsyncCallbackCompleted(-1));

WScript.Echo(Debug.msTraceAsyncCallbackStarting(opId7, -1));
WScript.Echo(Debug.msTraceAsyncCallbackCompleted(2));

WScript.Echo(Debug.msTraceAsyncCallbackStarting(opId5));
WScript.Echo(Debug.msTraceAsyncCallbackCompleted(1));

WScript.Echo(Debug.msTraceAsyncCallbackStarting(opId5, 1, 0));
WScript.Echo(Debug.msTraceAsyncCallbackCompleted(0, 0, 0, 0, 0));

WScript.Echo(Debug.msTraceAsyncCallbackStarting(opId2, 2, 5));
WScript.Echo(Debug.msTraceAsyncCallbackCompleted(1, 1, 1));

WScript.Echo(Debug.msUpdateAsyncCallbackRelation());
WScript.Echo(Debug.msUpdateAsyncCallbackRelation(opId4));
WScript.Echo(Debug.msUpdateAsyncCallbackRelation(opId2, -1));
WScript.Echo(Debug.msUpdateAsyncCallbackRelation(opId1, 0));
WScript.Echo(Debug.msUpdateAsyncCallbackRelation(opId3, 1, 1));
WScript.Echo(Debug.msUpdateAsyncCallbackRelation(opId3, 1, 1));
WScript.Echo(Debug.msUpdateAsyncCallbackRelation(opId8, 2, 2));
WScript.Echo(Debug.msUpdateAsyncCallbackRelation(opId5, -1, 1));
WScript.Echo(Debug.msUpdateAsyncCallbackRelation(opId6, undefined, 1));
WScript.Echo(Debug.msUpdateAsyncCallbackRelation(opId7, 2, 5));
