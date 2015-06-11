var Service1=function() {
Service1.initializeBase(this);
this._timeout = 0;
this._userContext = null;
this._succeeded = null;
this._failed = null;
}
Service1.prototype={
_get_path:function() {
 var p = this.get_path();
 if (p) return p;
 else return Service1._staticInstance.get_path();},
DoWork:function(succeededCallback, failedCallback, userContext) {
/// <param name="succeededCallback" type="Function" optional="true" mayBeNull="true"></param>
/// <param name="failedCallback" type="Function" optional="true" mayBeNull="true"></param>
/// <param name="userContext" optional="true" mayBeNull="true"></param>
return this._invoke(this._get_path(), 'DoWork',false,{},succeededCallback,failedCallback,userContext); },
totalCost:function(numberHours,price,succeededCallback, failedCallback, userContext) {
/// <param name="numberHours" type="Number">System.Int32</param>
/// <param name="price" type="Number">System.Decimal</param>
/// <param name="succeededCallback" type="Function" optional="true" mayBeNull="true"></param>
/// <param name="failedCallback" type="Function" optional="true" mayBeNull="true"></param>
/// <param name="userContext" optional="true" mayBeNull="true"></param>
return this._invoke(this._get_path(), 'totalCost',false,{numberHours:numberHours,price:price},succeededCallback,failedCallback,userContext); }}
Service1.registerClass('Service1',Sys.Net.WebServiceProxy);
Service1._staticInstance = new Service1();
Service1.set_path = function(value) {
Service1._staticInstance.set_path(value); }
Service1.get_path = function() { 
/// <value type="String" mayBeNull="true">The service url.</value>
return Service1._staticInstance.get_path();}
Service1.set_timeout = function(value) {
Service1._staticInstance.set_timeout(value); }
Service1.get_timeout = function() { 
/// <value type="Number">The service timeout.</value>
return Service1._staticInstance.get_timeout(); }
Service1.set_defaultUserContext = function(value) { 
Service1._staticInstance.set_defaultUserContext(value); }
Service1.get_defaultUserContext = function() { 
/// <value mayBeNull="true">The service default user context.</value>
return Service1._staticInstance.get_defaultUserContext(); }
Service1.set_defaultSucceededCallback = function(value) { 
 Service1._staticInstance.set_defaultSucceededCallback(value); }
Service1.get_defaultSucceededCallback = function() { 
/// <value type="Function" mayBeNull="true">The service default succeeded callback.</value>
return Service1._staticInstance.get_defaultSucceededCallback(); }
Service1.set_defaultFailedCallback = function(value) { 
Service1._staticInstance.set_defaultFailedCallback(value); }
Service1.get_defaultFailedCallback = function() { 
/// <value type="Function" mayBeNull="true">The service default failed callback.</value>
return Service1._staticInstance.get_defaultFailedCallback(); }
Service1.set_enableJsonp = function(value) { Service1._staticInstance.set_enableJsonp(value); }
Service1.get_enableJsonp = function() { 
/// <value type="Boolean">Specifies whether the service supports JSONP for cross domain calling.</value>
return Service1._staticInstance.get_enableJsonp(); }
Service1.set_jsonpCallbackParameter = function(value) { Service1._staticInstance.set_jsonpCallbackParameter(value); }
Service1.get_jsonpCallbackParameter = function() { 
/// <value type="String">Specifies the parameter name that contains the callback function name for a JSONP request.</value>
return Service1._staticInstance.get_jsonpCallbackParameter(); }
Service1.set_path("");
Service1.DoWork= function(onSuccess,onFailed,userContext) {
/// <param name="onSuccess" type="Function" mayBeNull="true"></param>
/// <param name="onFailed" type="Function" optional="true" mayBeNull="true"></param>
/// <param name="userContext" optional="true" mayBeNull="true"></param>
Service1._staticInstance.DoWork(onSuccess,onFailed,userContext); }
Service1.totalCost= function(numberHours,price,onSuccess,onFailed,userContext) {
/// <param name="numberHours" type="Number">System.Int32</param>
/// <param name="price" type="Number">System.Decimal</param>
/// <param name="onSuccess" type="Function" optional="true" mayBeNull="true"></param>
/// <param name="onFailed" type="Function" optional="true" mayBeNull="true"></param>
/// <param name="userContext" optional="true" mayBeNull="true"></param>
Service1._staticInstance.totalCost(numberHours,price,onSuccess,onFailed,userContext); }
