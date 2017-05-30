#pragma once

/// <summary>A list of opcodes supported/handled/inspected by telemetry code. Add an opcode to this interface on an
/// as-needed basis and add the call to it from within the handlers defined in InterpreterHandler.inl.</summary>
/// <remarks>This interface is not used for any polymorphic behaviour at runtime, but as a means of implementing a
/// compile-time "checklist" to ensure that Opcode telemetry providers' capabilities match the opcodes handled by
/// OpcodeTelemetry.</remarks>
__interface IOpcodeTelemetry // `__interface` has implied `__declspec(novtable)`.
{
    // These methods are sorted in no particular order. Feel free to re-arrange as logic or aesthetics dicate.

    /// <summary>`instance.property()`</summary>
    void GetMethodProperty(const Js::Var& instance, const Js::PropertyId& propertyId, const Js::Var& value, const bool successful) = 0;

    /// <summary>`if( instance.property )`</summary>
    void GetProperty(const Js::Var& instance, const Js::PropertyId& propertyId, const Js::Var& value, const bool successful) = 0;
    
    /// <summary>`instance instanceof constructorFunction`</summary>
    void IsInstanceOf(const Js::Var& instance, const Js::Var& constructorFunction, const Js::Var& result) = 0;
    
    /// <summary>`typeof instance.property` - note that a var can be a root-property. I need to research what `typeof localVar` evaluates to.</summary>
    void TypeOfProperty(const Js::Var& instance, const Js::PropertyId& propertyId, const Js::Var& value, const Js::Var& result, const bool successful) = 0;
    
    /// <summary>`if( "propertyName" in instance )` (but only for string propertyNames, integer indexes aren't collected - feel free to change this, if necessary).</summary>
    /// <remarks>It is currently indeterminate if the `propertyName` parameter is necessary (if the value can be properly derived from `propertyId`, then it will be removed)</remarks>
    void IsIn(const Js::Var& instance, const Js::PropertyId& propertyId, const bool successful) = 0;

    /// <summary>`var foo = new Foo();`</summary>
    /// <remarks>This call is made after the controller is invoked. Take care as the constructor might have mutated the arguments, also <paramref="constructedInstance" /> could be a JavascriptProxy instance wrapper rather than the originally constructed object.</remarks>
    void NewScriptObject(const Js::Var& constructorFunction, const Js::Arguments& arguments, const Js::Var& constructedInstance) = 0;
};