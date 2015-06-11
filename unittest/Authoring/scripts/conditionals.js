// Conditional Compilation Statements outside comments
@if (@_win32)
	@set @platform32 = 5
@elif (
	@_win16)
	@set @platform32 = false;
@elif (
	@_mac
	)
	@set @oswindows = false
@else
	@set @oswindows = true
@end

// Keywords as Conditional Compilation variables
@cc_on
@set @var = ((1 - 1) * 5)
@set @while = true

// Conditional Compilation Statements within comments
/*@cc_on @*/
/*@
@if (@_win32)
	@set @platform32 = 5
@elif (
	@_win16)
	@set @platform32 = false;
@end
@*/

// Conditional Compilation Statements with @_win32
@if (@_win32)
	var j = 20;
@else
	var i = 10;
@end

// Conditional Compilation Statements with @_win16
@if (@_win16)
	var j = 20;
@else
	var i = 10;
@end

// Conditional Compilation Statements with @_x86
@if (@_x86)
	var j = 20;
@else
	var i = 10;
@end

// Conditional Compilation Statements with @_jscript
@if (@_jscript)
	var j = 20;
@else
	var i = 10;
@end

// Conditional Compilation Statements with @_jscript_build
@if (@_jscript_build)
	var j = 20;
@else
	var i = 10;
@end

// Conditional Compilation Statements with @_jscript_version
@if (@_jscript_version)
	var j = 20;
@else
	var i = 10;
@end

// Conditional Compilation Statements with @_mac
@if (@_mac)
	var j = 20;
@else
	var i = 10;
@end

// Conditional Compilation Statements with @_alpha
@if (@_alpha)
	var j = 20;
@else
	var i = 10;
@end

// Conditional Compilation Statements with @_powerpc
@if (@_PowerPC)
	var j = 20;
@else
	var i = 10;
@end