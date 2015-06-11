use lib "$ENV{SDXROOT}\\inetcore\\mshtml\\types";
use Data::Dumper;
use FastDOM::SpecIDLParser;

my $flavor = shift @ARGV;
if ($flavor ne "windows" && $flavor ne "web" && $flavor ne "worker")
{
    die "Allowable flavors are 'windows', 'web', and 'worker' only.";
}

# Read and merge input files
my $inputFile = shift @ARGV;
# print "Processing: $inputFile\n";
my $additionalDefintions;
my $parseResult = &GetParseResult($inputFile);
if ($flavor eq "web" || $flavor eq "windows")
{
    while ($inputFile = shift @ARGV){
        # print "Processing: $inputFile\n";
        my $additionalParseResult = &GetParseResult($inputFile);
        &MergeVersionedFiles($parseResult, $additionalParseResult);
    }
    
    MergeAdditionalDefinitions($parseResult, "AdditionalDomDefinitions.pl");
}
elsif ($flavor eq "worker")
{
    $inputFile = shift @ARGV;
    # print "Processing: $inputFile\n";
    $additionalDefintions = &GetParseResult($inputFile);
    while ($inputFile = shift @ARGV){
        # print "Processing: $inputFile\n";
        my $additionalParseResult = &GetParseResult($inputFile);
        &MergeVersionedFiles($additionalDefintions, $additionalParseResult);
    }

    MergeAdditionalDefinitions($parseResult, "WebWorkersAdditionalDomDefinitions.pl");
    MergeAdditionalDefinitions($additionalDefintions, "AdditionalDomDefinitions.pl");
}
else
{
    die "Unknown flavor: $flavor";
}

# set the global object name
if (!defined $parseResult->{MODULE}->{ATTRIBUTES}->{MSGLOBALOBJECTPOLLUTOR}){
    die "Can not find global object definition \n";
}
my $globalObjectName = $parseResult->{MODULE}->{ATTRIBUTES}->{MSGLOBALOBJECTPOLLUTOR};

my $defaultEventName;
if ($flavor eq "worker") {
    $defaultEventName = "Event";
}
else {
    $defaultEventName = "MSEventObj";
}

sub EvalFile($)
{
    my $inputFile = shift;
    open IN, "<$inputFile";
    my @file = <IN>;
    my $file = join("", @file);
    close IN;
    $result = eval($file);
    return $result;
}

sub MergeAdditionalDefinitions
{
    my $parseResult = shift;
    my $additionalDefinitions = EvalFile(shift);

    sub MergeHashes {
        my ($x, $y) = @_;
        foreach my $k (keys %$y) {
            if (!defined($x->{$k})) {
                $x->{$k} = $y->{$k};
            } else {
                $x->{$k} = MergeHashes($x->{$k}, $y->{$k});
            }
        }
        return $x;
    }	
    MergeHashes($parseResult, $additionalDefinitions);
}

sub GetParseResult($) 
{
    my $parseResult;
    my $inputFile = shift;
    
    # parse the specIDL, store the parsing result in a file
    my $parseResultFile = "${inputFile}.parsed";

    sub GeneratedFileUpToDate
    {
        my $inputFile = shift;
        my $parseResultFile = shift;

        # print "inputFile = $inputFile \n";
        # print "parseResultFile = $parseResultFile \n";

        if (-e $parseResultFile) {
            my $outputFileTimestamp = ( lstat $parseResultFile )[ 9 ];
            my $inputFileTimestamp = ( lstat $inputFile )[ 9 ];
            
            # print "timestamp for inputFile = $inputFileTimestamp \n";
            # print "timestamp for outputFile = $outputFileTimestamp \n";
            if ($outputFileTimestamp > $inputFileTimestamp)
            {
                # print "outputFile is up-to-date \n";
                return 1;
            }
            # print "outputFile is stale \n";
        }
        # print "outputFile does not exisit \n";
        return 0;
    }

    unless (-e $inputFile) {
        die "Could not find input file: $inputFile \n";
    }

    if (GeneratedFileUpToDate($inputFile, $parseResultFile) == 0 ) {
        # print "Generating .parsed file \n";
        $parseResult = &ParseFile($inputFile);
        open OUT, ">$parseResultFile";
        print OUT Dumper($parseResult);
        close OUT;
    }
    
    return EvalFile($parseResultFile);
}

# Method copied from fastDOMCompiler.pl
sub MergeVersionedFiles()
{
    my $VAR1;
    my $rootParseResult = shift;

    # print "Processing: $rootParseResult->{MODULE}{NAME}\n";		
    while(my $mergeResult = shift)
    {
        # print "Processing: $mergeResult->{MODULE}{NAME}\n";		
        if ( !defined $mergeResult->{MODULE}{ATTRIBUTES}{MSBROWSERVERSION} )
        {
            die "$mergeResult->{MODULE}{NAME} needs an msBrowserVersion attribute to be merged";
        }
            
        my $configurableTypeSystemToken = $mergeResult->{MODULE}{ATTRIBUTES}{MSBROWSERVERSION};
        for my $interfaceName ( sort keys %{$mergeResult->{INTERFACES}} )
        {
            my $sourceInterface = $mergeResult->{INTERFACES}{$interfaceName};
            if ( defined $rootParseResult->{INTERFACES}{$interfaceName} )
            {
                my $targetInterface = $rootParseResult->{INTERFACES}{$interfaceName};
        
                # merge an existing interface with some validation
                if ( !defined $sourceInterface->{ATTRIBUTES}{SUPPLEMENTAL} )
                {
                    die "A non supplemental interface cannot be merged with an existing interface, $interfaceName";
                }
                
                # Move over any new implements which we inherited from the supplemental interface.
                # CONSIDER: Disallowing implements from new supplementals for old interfaces (HTMLElement implements 9ModeAPI, but only in 10Mode)
                if ( defined $sourceInterface->{IMPLEMENTS} )
                {
                    for my $implementsName ( keys %{$sourceInterface->{IMPLEMENTS}} )
                    {
                        $targetInterface->{IMPLEMENTS}{$implementsName} = 1;
                    }
                }

                # Move over all Properties, Methods and Constants marking them as they come over.
                my @groups = ( "PROPERTY", "CONSTANT", "METHOD" );
                for my $group (@groups)
                {
                    if ( defined $sourceInterface->{$group} )
                    {
                        my $groupHash = $sourceInterface->{$group};
                        for my $memberKey (sort keys %{$groupHash})
                        {
                            if ( defined $targetInterface->{$group}{$memberKey} )
                            {
                                die "$targetInterface->{NAME}:$memberKey $group already exists on target interface";
                            }

                            my $member = $groupHash->{$memberKey};
                            $member->{ATTRIBUTES}{MSBROWSERVERSION} = $configurableTypeSystemToken;
                            $targetInterface->{$group}{$memberKey} = eval(Dumper($member));
                        }
                    }
                }

                # Move over all relevant attributes
                my @attributeGroups = ( "MSRELATEDEVENT", "MSRELATEDELEMENT" );
                for my $group (@attributeGroups)
                {
                    if ( defined $sourceInterface->{ATTRIBUTES}{$group} )
                    {
                        my $groupHash = $sourceInterface->{ATTRIBUTES}{$group};
                        for my $memberKey (sort keys %{$groupHash})
                        {
                            my $member = $groupHash->{$memberKey};
                            $targetInterface->{ATTRIBUTES}{$group}{$memberKey} = eval(Dumper($member));
                        }
                    }
                }

            }
            else
            {
                # import a new interface with some validation
                if ( defined $sourceInterface->{ATTRIBUTES}{SUPPLEMENTAL} )
                {
                    die "A supplemental interface cannot be imported as an initial interface";
                }
                $sourceInterface->{ATTRIBUTES}{MSBROWSERVERSION} = $configurableTypeSystemToken;
                $rootParseResult->{INTERFACES}{$interfaceName} = eval(Dumper($sourceInterface));
            }
        }
    }
}

sub InterfaceList {
    return $parseResult->{INTERFACES};
}

sub GetInterfaceByName {
    my $interfaceName = @_[0];
    if (!defined $parseResult->{INTERFACES}->{$interfaceName})
    {
        #print "could not find type $interfaceName";
        return;
    }
    return InterfaceList()->{$interfaceName};	
}

sub HideUnneededTypesAndProperties {
    sub ClearTypeMembers {
        my $typeName = @_[0];
        my $type = GetInterfaceByName($typeName); 
        if (defined $type) {
            delete $type->{METHOD};
            delete $type->{PROPERTY};
            delete $type->{CONSTANT};
        }
    }

    sub ShouldDisable {
        my $object = @_[0];
        sub ShouldDisableTag {
            # implementation based on s_IsHostBehaviorSupported method in inetcore\mshtml\src\site\base\formkrnl.hxx
            my $tag = shift;
            if ($tag eq "DISABLEDFORIE10" ||
                          $tag eq "DISABLEDFORIE11")                { return 1;} # always disable for non ie10 features
            elsif ($tag eq "DisableInWebWorker")                    { return $flavor eq "worker" ? 1 : 0; } # disable only for WebWorkers
            elsif ($tag eq "OnlyForWwaLocalCompartment" ||
                          $tag eq "DISABLEDFORIE11ENABLEDINWWA")    { return $flavor eq "windows" ? 0 : 1; } # enable only for WWA
            elsif ($tag eq "HostFlagAlert" ||
                          $tag eq "HostFlagAnchorUrn" ||
                          $tag eq "HostFlagBehaviors" ||
                          $tag eq "HostFlagBrowserHostInfo" ||
                          $tag eq "HostFlagNavigatorPlugins" ||
                          $tag eq "HostFlagHelpDialog" ||
                          $tag eq "HostFlagMimeTypes" ||
                          $tag eq "HostFlagMSFileSaver" ||
                          $tag eq "HostFlagMSProtocols" ||
                          $tag eq "HostFlagResponseBody" ||
                          $tag eq "HostFlagShowWindow" ||
                          $tag eq "HostFlagUserConfirm" ||
                          $tag eq "HostFlagUserPrompt" ||
                          $tag eq "HostFlagWindowMoveResize" ||
                          $tag eq "HostFlagMSLaunchUri" ||
                          $tag eq "HostFlagCreatePopup")            { return $flavor eq "windows" ? 1 : 0; } #disable for WWA
            elsif ($tag eq "HostFlagIndexedDB" ||
                          $tag eq "HostFlagAppCache" ||
                          $tag eq "HostFlagDOMStorage" ||
                          $tag eq "DisabledForSandbox" ||
                          $tag eq "DisabledForWin7" ||
                          $tag eq "DISABLEDFORWWA" ||
                          $tag eq "DISABLEFORIEUSERAGENT" ||
                          $tag eq "OnlyForScriptDebugMode" ||
                          $tag eq "FEATURE_WEBSOCKET" ||
                          $tag eq "FEATURE_XDOMAINREQUEST"||
                          $tag eq "HOSTFLAGDISABLEGEOLOCATION" ||
                          $tag eq "OnlyForWwaInIE10EnableForIE11")  { return 0; } # do not disable for these tags
            else                                                    { die "Unknown msInternalDisableKey value: $tag"; }
        }
        my $msInternalDisableKey = $object->{ATTRIBUTES}->{MSINTERNALDISABLEKEY};
        if ($msInternalDisableKey ne "") {
            my @tags = split(/,/,$msInternalDisableKey);
            for my $tag (@tags) {
                # Delete IE10 disabled methods, WWA types for Web, and Web only types for WWA
                my $result = ShouldDisableTag($tag);
                if ($result == 1) {
                    return 1;
                }
            }
        }
        return 0;
    }
    ClearTypeMembers('MSResourceMetadata');
    ClearTypeMembers('MSHTMLCollectionExtensions');
    ClearTypeMembers('MSStorageExtensions');
    if (defined GetInterfaceByName('Document')) {
        delete GetInterfaceByName('Document')->{PROPERTY}->{'Script'};
    }

    for my $interfaceName ( keys %{$parseResult->{INTERFACES}} ) {
        my $interface = $parseResult->{INTERFACES}{$interfaceName};
        if (ShouldDisable($interface) == 1) {
            #print "deleteing interface $interfaceName \n";
            delete $parseResult->{INTERFACES}{$interfaceName};
        } else {
            for my $propkey ( keys %{$interface->{PROPERTY}} ) {
                my $prop = $interface->{PROPERTY}{$propkey};
                if (ShouldDisable($prop) == 1) {
                    #print "deleteing property $propkey \n";
                    delete $interface->{PROPERTY}{$propkey};
                }
            }
            for my $methodkey ( keys %{$interface->{METHOD}} ) {
                my $method = $interface->{METHOD}{$methodkey};
                if (ShouldDisable($method) == 1) {
                    #print "deleteing method $methodkey \n";
                    delete $interface->{METHOD}{$methodkey};
                }
            }
            for my $constantkey ( keys %{$interface->{CONSTANT}} ) {
                my $constant = $interface->{CONSTANT}{$constantkey};
                if (ShouldDisable($constant) == 1) {
                    #print "deleteing constant $constantkey \n";
                    delete $interface->{CONSTANT}{$constantkey};
                }
            }
        }
    }
}
# Hide unneeded interfaces & properties
HideUnneededTypesAndProperties();


# Some parameter names are not valid in JavaScript
sub AdjustParamName {
    my $paramName = @_[0];
    # note: parameter cannot be named "default" in JavaScript so we need to rename it.
    if($paramName eq "default") {
        return "defaultValue";
    }
    return $paramName;
}

my $DOM_TO_JS_TYPE_MAP = {
    'DOMString' => 'String',
    'DOMTimeStamp' => 'Number',
    'DOMHighResTimeStamp' => 'Number',
    'bool' => 'Boolean',
    'boolean' => 'Boolean',
    'Boolean' => 'Boolean',
    'float' => 'Number',
    'double' => 'Number',
    'long' => 'Number',
    'short' => 'Number',
    'unsigned short' => 'Number',
    'signed short' => 'Number',
    'unsigned long' => 'Number',
    'signed long' => 'Number',
    'unsigned long long' => 'Number',
    'signed long long' => 'Number',
    'Function' => 'Function',
    'EventHandler' => 'Function',
    'void' => 'undefined',
    'any' => 'Object',
    'object' => 'Object',
    'Uint8Array' => 'Uint8Array',
    'ReadyState' => 'Number',
    'EndOfStreamError' => 'Number',
    'Date' => 'Date',
    'ArrayBuffer' => 'ArrayBuffer',
    'ArrayBufferView' => 'Uint8Array',
    'AbortMode' => 'String',
    'UnrestrictedDouble' => 'Number',
    'Float32Array' => 'Float32Array',
    'Int32Array' => 'Int32Array'
};

my $JS_PRIMITIVE_DEFAULT_VALUES = {
    'Number' => '0',
    'Boolean' => 'false',
    'String' => "''",
    'Function' => 'function() {}',
    'Object' => '{}',
    'undefined' => 'undefined'
};

sub DomToJsType {
    my $domtype = @_[0];

    my $result = $DOM_TO_JS_TYPE_MAP->{$domtype};
    if ($result) { 
        return $result; 
    }

    if($parseResult->{INTERFACES}{$domtype}) {
        return $domtype;
    }

    my $prefix = substr($domtype, 0, 9);
    if ($prefix eq "sequence<")
    {
        # If this type is a sequence<> type, it is a JS array
        return "Array";
    }

    if ($flavor eq "worker")
    {
        #print "\n// ==> Ignoring type: $domtype \n";
        return "Object";
    }

    print "Cannot convert type " . $domtype . " to JavaScript type";
    die;
}

sub ElementTypeForArray {
    my $domtype = @_[0];

    # Currently, we only return an elementType if domType is a sequence
    my $prefix = substr($domtype, 0, 9);
    if ($prefix eq "sequence<")
    {
        # If this type is a sequence<> type, return the JsType of its generic parameter
        my $domElementType = substr($domtype, 9, -1);
        my $result = DomToJsType($domElementType);
        if ($result) {
            return $result;
        }
    }

    return;
}

sub JsDefaultValue
{
    my $jstype = @_[0];

    if(exists $JS_PRIMITIVE_DEFAULT_VALUES->{$jstype}) {
        return $JS_PRIMITIVE_DEFAULT_VALUES->{$jstype};
    }

    # Many methods and properties are declared to return Element but in fact return HTMLElement-derived objects. 	
    if($jstype eq "Element") {
        $jstype = "HTMLElement";
    }

    if(GetInterfaceByName($jstype)) {
        # return the name of a variable of the specified type.
        return $jstype;
    }
    
    return 'new ' . $jstype . '()';
}

sub IsArrayType {
    my $type = @_[0];
    my $objectName = $type->{NAME};
    my $itemMethod = $type->{METHOD}->{item};
    return ((($objectName =~ m/Collection$/) or ($objectName =~ m/List$/)) and $itemMethod);
}

sub IsEvent {
    my $prop = @_[0];
    return $prop->{PROPTYPE} eq "EventHandler";
}

sub DumpEvents
{
    my $typeName = @_[0];
    my $interface = @_[1];
    my @events = ();
    for my $propkey ( keys %{$interface->{PROPERTY}} ) {
        my $prop = $interface->{PROPERTY}{$propkey};
        if(IsEvent($prop)) {
            push @events, "\"$propkey\"";
        }
    }
    if(scalar(@events)) {
        my $events = join(", ", @events);
        print "\n\t_events($typeName, $events);"
    }
}

# Checks if a $type depends on $other type
sub IsDependsOn
{
    # Checks if $typeName is a super of $type
    sub IsSuper
    {
        my $type = @_[0];
        my $typeName = @_[1];
        
        if($type->{SUPER} eq "Object") {
            return 0;
        }
        if($type->{SUPER} eq $typeName) {
            return 1;
        }
        return IsSuper(GetInterfaceByName($type->{SUPER}), $typeName);
    }

    # Checks if $type implements $typeName
    sub IsImplements
    {
        my $type = @_[0];
        my $typeName = @_[1];
        
        if($type->{IMPLEMENTS}{$typeName}) {
            return 1;
        }
        return 0; 
    }

    my $type = @_[0];
    my $other = @_[1];
    if(IsSuper($type, $other->{NAME})) { return 1; }
    if(IsImplements($type, $other->{NAME})) { return 1; }
    return 0;
}

sub IsPrimitiveType {
    my $domtypeName = @_[0];
    if(defined $DOM_TO_JS_TYPE_MAP->{$domtypeName}) { 
        return 1; 
    }
    else {
        return 0;
    }
}

sub DumpProperties
{
    my $typeName = @_[0];
    my $interface = @_[1];
    for my $propkey ( keys %{$interface->{PROPERTY}} ) {
        my $prop = $interface->{PROPERTY}{$propkey};
        my $domtype = $prop->{PROPTYPE};
        my $value;
        if(defined $prop->{VALUE}) 
        {
                $value = $prop->{VALUE};
        }
        else
        {
            my $jstype = DomToJsType($domtype);
            $value = JsDefaultValue($jstype);
        }
        my $additionalStatements = "";
        if(not IsEvent($prop)) {
            print "\n\t" ;
            if ($typeName eq "Node" && $propkey eq "nodeType")
            {
                print $typeName . "." . $propkey . " = " . "1"; #Node.ELEMENT_NODE
            }
            elsif ($propkey eq "ownerDocument" || $propkey eq "contentDocument")
            {
                print $typeName . "." . $propkey . " = " . "document";
            }
            elsif ($propkey eq "firstChild")
            {
                print "Object.defineProperty($typeName,\"$propkey\", { get: function () { return _firstChild(this, $value); } })";
            }
            elsif ($propkey eq "lastChild")
            {
                print "Object.defineProperty($typeName,\"$propkey\", { get: function () { return _lastChild(this, $value); } })";
            }
            elsif ($propkey eq "childNodes")
            {
                print "Object.defineProperty($typeName,\"$propkey\", { get: function () { return _childNodes(this, $value); } })";
            }
            elsif ($propkey eq "firstElementChild")
            {
                print "Object.defineProperty($typeName,\"$propkey\", { get: function () { return _firstElementChild(this, $value); } })";
            }
            elsif ($propkey eq "lastElementChild")
            {
                print "Object.defineProperty($typeName,\"$propkey\", { get: function () { return _lastElementChild(this, $value); } })";
            }
            elsif ($propkey eq "nextElementSibling")
            {
                print "Object.defineProperty($typeName,\"$propkey\", { get: function () { return _nextElementSibling(this, $value); } })";
            }
            elsif ($propkey eq "previousElementSibling")
            {
                print "Object.defineProperty($typeName,\"$propkey\", { get: function () { return _previousElementSibling(this, $value); } })";
            }
            elsif ($propkey eq "parentElement")
            {
                print "Object.defineProperty($typeName,\"$propkey\", { get: function () { return _parentElement(this, $value); } })";
            }
            elsif ($propkey eq "childElementCount")
            {
                print "Object.defineProperty($typeName,\"$propkey\", { get: function () { return _childElementCount(this); } })";
            }
            elsif ($propkey eq "innerHTML")
            {
                print "Object.defineProperty($typeName,\"$propkey\", { get: function () { return ''; }, set: function (v) { _setInnerHTML(this, v); } })";
            }
            elsif ($propkey eq "images")
            {	
                print $typeName . "." . $propkey . " = _createHTMLCollection('img')";
            }
            elsif (($typeName eq 'HTMLTableElement' or $typeName eq 'HTMLTableSectionElement') and $propkey eq "rows")
            {	
                print $typeName . "." . $propkey . " = _createHTMLCollection('tr')";
            }
            elsif ($typeName eq 'HTMLTableRowElement' and $propkey eq "cells")
            {	
                print $typeName . "." . $propkey . " = _createHTMLCollection('td')";
            }
            elsif ($typeName eq 'Document' and $propkey eq "scripts")
            {	
                print $typeName . "." . $propkey . " = _createHTMLCollection('script')";
            }
            elsif ($typeName eq 'Document' and $propkey eq "applets")
            {	
                print $typeName . "." . $propkey . " = _createHTMLCollection('applet')";
            }
            elsif ($typeName eq 'Document' and $propkey eq "forms")
            {	
                print $typeName . "." . $propkey . " = _createHTMLCollection('form')";
            }
            elsif ($typeName eq 'Document' and ($propkey eq "links" or $propkey eq "anchors"))
            {	
                print $typeName . "." . $propkey . " = _createHTMLCollection('a')";
            }
            elsif ($typeName eq 'Document' and $propkey eq "embeds")
            {	
                print $typeName . "." . $propkey . " = _createHTMLCollection('embed')";
            }
            elsif ($typeName eq 'HTMLFormElement' and $propkey eq "elements")
            {	
                print "Object.defineProperty($typeName,\"$propkey\", { get: function () { return _formElements(this); } })";
            }
            elsif ($typeName eq 'HTMLSelectElement' and $propkey eq "options")
            {	
                print "Object.defineProperty($typeName,\"$propkey\", { get: function () { return _selectOptions(this); } })";
            }
            elsif ($typeName eq $value || (!IsPrimitiveType($domtype) && defined GetInterfaceByName($domtype) && IsDependsOn(GetInterfaceByName($domtype), $interface)))
            {
                print $typeName . "." . $propkey . " = " ."_\$getTrackingNull(Object.create($value))";
            }
            else
            {
                print $typeName . "." . $propkey . " = " . $value;
            }
            print ";";
        }
    }
}

sub DumpConstants
{
    my $typeName = @_[0];
    my $interface = @_[1];
    for my $propkey ( keys %{$interface->{CONSTANT}} ) {
        my $prop = $interface->{CONSTANT}{$propkey};
        my $value = $prop->{CONST_VALUE};

        print "\n\t" ;
            print $typeName . "." . $propkey . " = " . $value;
        print ";";
        }
}

sub DumpMethods
{
    sub OverloadsRequired {
        my $method = @_[0];

        my @params = @{$method->{PARAMETERS}};
        my $overloadedParamCount = 0;
        for my $param (@params) {
            if($param->{ATTRIBUTES}{APPLICABLE_TYPES})
            {
                $overloadedParamCount++;
            }
        }
        return $overloadedParamCount;
    }

    sub PostionOfFirstOverloadedParam {
        my $method = @_[0];

        my @params = @{$method->{PARAMETERS}};
        my $paramIndex = 0;
        for my $param (@params) {
            if($param->{ATTRIBUTES}{APPLICABLE_TYPES})
            {
                return $paramIndex;
            }
            $paramIndex++;
        }
        return -1;
    }
    
    sub DumpMethod {
            my $method = @_[0];
            my $objectName = @_[1];
            my $retDomType = $method->{RETURN_TYPE};
            my $methodName = $method->{NAME};
            my $jsRetType = DomToJsType($retDomType);
            my @params = @{$method->{PARAMETERS}};
            my $paramCount = scalar(@params);
            my $returns = not ($jsRetType eq "undefined");
            my $includeDocComments = (@params or $returns);
    
            sub DumpSignatureDocComment
            {
                my $method = @_[0];
                my @params = @{$method->{PARAMETERS}};
                my $returns = not (DomToJsType($method->{RETURN_TYPE}) eq "undefined");
                print "\n\t\t/// <signature>\n";
                if(@params) {
                    for my $param (@params) {
                        my $paramName = AdjustParamName($param->{NAME});
                        my $paramType = DomToJsType($param->{PARAMETER_TYPE});
                        my $isOptional = 0;
                        print "\t\t/// <param name='" . $paramName . "' type='" . $paramType . "'";
                        if ($paramType eq "Array") {
                            my $elementType = ElementTypeForArray($param->{PARAMETER_TYPE});
                            if ($elementType) {
                                print " elementType='" . $elementType . "'";
                            }
                        }
                        if($param->{optional} eq '1') { 
                            print " optional='true'";
                        }
                        print " />\n";
                    }
                }
                my $retType = DomToJsType($method->{RETURN_TYPE});
                if($returns) {
                    print "\t\t/// <returns type='" . $retType . "'";
                    if ($retType eq "Array") {
                        my $elementType = ElementTypeForArray($method->{RETURN_TYPE});
                        if ($elementType) {
                            print " elementType='" . $elementType . "'";
                        }
                    }
                    print "/>\n";
                }
                print "\t\t/// </signature>";
            }
            
            # Create a function declaration
            print "\n\t" . $objectName . "." . $methodName . " = function(";
            my $firstParam = 1;
            my @params = @{$method->{PARAMETERS}};
            for my $param (@params) {
                if(not $firstParam) {
                    print ", ";
                }
                print AdjustParamName($param->{NAME}); 
                $firstParam = 0;
            }
            print ") { ";
            
            if($includeDocComments) {
                # When parameters or return type is available, include doc comments
                $overloadedParameters = OverloadsRequired($method);
                if($overloadedParameters > 0) {
                    my $overloads = $method->{OVERLOADS};
                    if($overloads) {
                        # Dump method overloads
                        for my $overload ( @{$overloads} ) {
                            DumpSignatureDocComment($overload);
                        }
                    }
                    else {
                        if($overloadedParameters > 1) {
                            die "Overloads must be specified for $objectName.$methodName\n";
                        }
                        my $overloadParamPosition = PostionOfFirstOverloadedParam($method);
                        if ($overloadParamPosition < 0) {
                            die "Expected overloaded parameter in method signature requiring overloads\n"
                        }
                        my $param = @params[$overloadParamPosition];
                        # Keep the base type
                        my $baseType = $param->{'PARAMETER_TYPE'};
                        for my $type (keys %{$param->{ATTRIBUTES}{APPLICABLE_TYPES}}) {
                            $param->{'PARAMETER_TYPE'} = $type;
                            DumpSignatureDocComment($method);
                        }
                        # Restore the base type
                        $param->{'PARAMETER_TYPE'} = $baseType;
                    }
                } else {
                    DumpSignatureDocComment($method);
                }
            }
            
            if(defined $method->{BODY}) {
                print "\n\t\t" . $method->{BODY};
                $returns = 0;
            }

            if($returns) {
                if($includeDocComments) { 
                    print "\n\t\t";
                }
                if ($methodName eq "item" && IsArrayType(GetInterfaceByName($objectName))) {
                    if (!IsPrimitiveType($method->{RETURN_TYPE})){
                        print "return this[index] || _\$getTrackingNull(Object.create(". JsDefaultValue($jsRetType) . ")); ";
                    }
                    else {
                        print "return this[index] || _\$getTrackingNull(". JsDefaultValue($jsRetType) . "); ";
                    }
                }
                else {
                    print "return " . JsDefaultValue($jsRetType) . "; ";
                }
            }

            if($includeDocComments) { 
                print "\n\t";
            }

            print "};";
        }

    my $objectName = @_[0];
    my $interface = @_[1];

    for my $methodName ( keys %{$interface->{METHOD}} ) {
        my $method = $interface->{METHOD}{$methodName};
        DumpMethod($method, $objectName);
    }
}

sub DumpTypeDecl {
    my $objectName = @_[0];
    my $type = @_[1];
    print "\tvar " . $objectName . " = ";
    if($objectName eq $globalObjectName) {
        print "this";
    } else {
        if($type->{SUPER} eq "Object") { print "{}"; } 
        else { print "_\$inherit(" . $type->{SUPER} .")"; }
    }
    print ";\n";
    if($type->{ATTRIBUTES}->{CONSTRUCTOR}) {
        print "\tvar " . $objectName . "Ctor = function() { return Object.create($objectName); };\n";
    }
}

sub DumpInitializations {
    my $objectName = @_[0];
    my $type = @_[1];

    if ($type->{NAME} eq "Text") {
        print"\n\t$objectName.nodeType = Node.TEXT_NODE;";
        print"\n\t$objectName.nodeName = '#text';";
    }
    elsif ($type->{NAME} eq "Comment") {
        print"\n\t$objectName.nodeType = Node.COMMENT_NODE;";
        print"\n\t$objectName.nodeName = '#comment';";
    }
    elsif ($type->{NAME} eq "CDATASection") {
        print"\n\t$objectName.nodeType = Node.CDATA_SECTION_NODE;";
        print"\n\t$objectName.nodeName = '#cdata-section';";
    }
    elsif ($type->{NAME} eq "ProcessingInstruction") {
        print"\n\t$objectName.nodeType = Node.PROCESSING_INSTRUCTION_NODE;";
    }
    elsif ($type->{NAME} eq "DocumentFragment") {
        print"\n\t$objectName.nodeType = Node.DOCUMENT_FRAGMENT_NODE;";
        print"\n\t$objectName.nodeName = '#document-fragment';";
    }
    elsif ($type->{NAME} eq "DocumentType") {
        print"\n\t$objectName.nodeType = Node.DOCUMENT_TYPE_NODE;";
        print"\n\t$objectName.nodeName = 'html';";
    }
    elsif (defined $type->{ATTRIBUTES}->{MSRELATEDELEMENT}) {
        my @names = keys %{$type->{ATTRIBUTES}->{MSRELATEDELEMENT}};
        if (defined @names[0]) {
            print"\n\t$objectName.nodeName = $objectName.tagName = '". uc(@names[0]) ."';";
            print"\n\t$objectName.localName = '". lc(@names[0]) ."';";
        }
    }
}

sub DumpTypeInit {
    my $objectName = @_[0];
    my $type = @_[1];
    
    my $typeName = $type->{NAME};

    print "\n\t/* -- type: " . $objectName . " -- */\n";

    # Implemented interfaces
    for my $implementedInterfaceName ( keys %{$type->{IMPLEMENTS}} ) {
        print "\t_\$implement($objectName, $implementedInterfaceName);\n"; 
    }
    
    if($type->{ATTRIBUTES}->{CONSTRUCTOR}) {
        DumpConstants($objectName . 'Ctor', $type);
    }
    DumpProperties($objectName, $type);
    DumpConstants($objectName, $type);
    DumpMethods($objectName, $type);
    DumpInitializations($objectName, $type);
    DumpEvents($objectName, $type);

    if(IsArrayType($type)) {
        my $arrayElementType = "null";
        my $itemMethod = $type->{METHOD}->{item};
        if($itemMethod->{RETURN_TYPE}) {
            $arrayElementType = JsDefaultValue(DomToJsType($itemMethod->{RETURN_TYPE}));
        }
        print "\n\t/* Add a single array element */\n";
        #print "\t" . $objectName . "[0] = _\$getTrackingNull(Object.create(" . $arrayElementType . "));";
        if (!IsPrimitiveType($itemMethod->{RETURN_TYPE})){
            print "\t" . $objectName . "[0] = _\$getTrackingNull(Object.create(" . $arrayElementType . "));";
        }
        else{
            print "\t" . $objectName . "[0] = _\$getTrackingNull(" . $arrayElementType . ");";
        }
    }
    
    print "\n\n"; # end type initialization
}

sub SortTypes {
    # Sorts the types based on their interdependencies using topological sort.
    sub FindTypeWithNoDependents {
        sub HasDependents	{
        
            my $type = @_[0];
            my $types = @_[1];
            for (my $i = 0; $i < @$types; $i++ ) {
                if(IsDependsOn($types->[$i], $type)) { 
                    return 1; 
                }
            }
            return 0;
        }	
        my $types = @_[0];
        for ( my $i = 0; $i < @$types; $i++ ) {
            if(!HasDependents($types->[$i], $types)) { 
                return $i; 
            }
        }
        if($#$types == 1) { return 0; }
        return -1;
    }

    my $array = @_[0];
    my @queue = @$array;
    my @result = ();	
    while(scalar(@queue)) {
        my $i = FindTypeWithNoDependents(\@queue);
        if($i < 0) { die "Cyclic dependencies between types detected."; }
        my $type = @queue[$i];
        splice(@queue, $i, 1); # remove from the queue
        unshift(@result, $type); # add to the sorted results array 
    }
    
    return @result;
}

sub DumpPublicInterfaces {
    my @interfacesObjects = values %{$parseResult->{INTERFACES}};

    for my $interface ( @interfacesObjects ) {
        my $interfaceName = $interface->{NAME};
        if ($interface->{ATTRIBUTES}->{MSINTERFACEOBJECTONLY} == 1) {
            # Expose the object directelly if it is a 'msInterfaceObjectOnly'
            print "\t_publicObject('$interfaceName', $interfaceName);\n";
        }
        elsif ($interface->{ATTRIBUTES}->{NOINTERFACEOBJECT} != 1 && $interface->{ATTRIBUTES}->{MSFOREIGNOBJECT} != 1 && ! $interface->{ATTRIBUTES}->{CALLBACK} && ! $interface->{ATTRIBUTES}->{CONSTRUCTOR} ) {
            print "\t_publicInterface('$interfaceName', ";
            print "{";

             my $firstMember = 1;

            # Expose constants
            my $firstConstant = 1;
            for my $propkey ( keys %{$interface->{CONSTANT}} ) {
                my $prop = $interface->{CONSTANT}{$propkey};
                my $value = $prop->{CONST_VALUE};
                if (not $firstMember) {
                    print ",";
                }
                $firstMember = 0;
                print "\n\t\t'$propkey' : $value";
            }

            # Expose static methods
            for my $methodName ( keys %{$interface->{METHOD}} ) {
                my $method = $interface->{METHOD}{$methodName};
                if ($method->{STATIC} == 1) {
                    if (not $firstMember) {
                        print ",";
                    }
                    $firstMember = 0;
                    print "\n\t\t'$methodName' : $interfaceName ['$methodName']";
                } 
            }

            if ($firstMember != 1) {
                print "\n\t"; 
            }

            print "}, $interfaceName);\n";
        }
    }
    print "\n";
}

sub DumpConstructors {
    my @interfacesObjects = values %{$parseResult->{INTERFACES}};

    for my $interface ( @interfacesObjects ) {
        my $interfaceName = $interface->{NAME};
        if ($interface->{ATTRIBUTES}->{CONSTRUCTOR}) {
            print "\t_publicInterface('$interfaceName', $interfaceName" . "Ctor , $interfaceName);\n";			
        }
    }
    print "\n";
}

sub CopyInterface {
    my $interfaceName = shift;
    my $sourceList = shift;
    my $destinationList = shift;
    
    if (! defined $sourceList->{INTERFACES}->{$interfaceName})
    {
        die "Could not find interface: $interfaceName in source lis. \n";
    }
    my $sourceInterface = $sourceList->{INTERFACES}->{$interfaceName};
    $destinationList->{INTERFACES}{$interfaceName} = eval(Dumper($sourceInterface));
}

sub EnsureClosure {
    my $WebWorkerIgnoredList = {
        'Window',
        'MSWindowExtensions',
        'Document',
        'Node',
        'Element'
    };

    sub IsIgnored {
        my $interfaceName = shift;
        if (exists $WebWorkerIgnoredList->{$interfaceName}){
            return 1;
        }
        else {
            return 0;
        }
    }
    
    sub LookupInsertType {
        my $interfaceName = @_[0];
        if (IsIgnored($interfaceName))
        {
            #print "Interface $interfaceName is ignored \n";
            return 0;
        }
        if (IsPrimitiveType($interfaceName) == 0 && !defined $parseResult->{INTERFACES}->{$interfaceName}) {
            # print "type: $interfaceName was NOT found in the list!!\n";
            if (!defined $additionalDefintions->{INTERFACES}->{$interfaceName}) {
                die "Unable to find type $interfaceName \n";
            }
            # copy the interface over
            CopyInterface($interfaceName, $additionalDefintions, $parseResult);
            return 1;
        }
        # interface is already in the list or is primitive
        return 0;
    }

    my $change;
    do
    {
        $change = 0;
        my @typeObjects = values %{$parseResult->{INTERFACES}};
        for my $type (@typeObjects) {
            my $typeName = $type->{NAME};
            #print "processing type: $typeName \n";

            if (defined $type->{SUPER} && $type->{SUPER} ne "Object") {
                if (LookupInsertType($type->{SUPER})) {
                        $change = 1;
                }
            }

        for my $name ( keys %{$type->{IMPLEMENTS}} )
            {
                # print "	 implements: $name\n";
                if (LookupInsertType($name)) {
                    $change = 1;
                }
            }

            for my $key ( keys %{$type->{PROPERTY}} ) {
                my $value = $type->{PROPERTY}{$key};
                # print "	 property: $typeName.".$value->{NAME} ."\n";
                if (LookupInsertType($value->{PROPTYPE})) {
                    $change = 1;
                }
            }

            for my $key ( keys %{$type->{CONSTANT}} ) {
                my $value = $type->{CONSTANT}{$key};
                # print "	 constant: $typeName.".$value->{NAME} ."\n";
                if (LookupInsertType($value->{CONST_TYPE})) {
                    $change = 1;
                }
            }

            for my $key ( keys %{$type->{METHOD}} ) {
                my $method = $type->{METHOD}{$key};
                # print "	 method: $typeName.".$method->{NAME} ."\n";
                if (LookupInsertType($method->{RETURN_TYPE})) {
                    $change = 1;
                }
                my @params = @{$method->{PARAMETERS}};
                for my $param ( @params) {
                    if (LookupInsertType($param->{PARAMETER_TYPE})) {
                        $change = 1;
                    }
                }
            }

        }
    } while ($change != 0);
}

sub DumpTypes {
    my @typeObjects = values %{$parseResult->{INTERFACES}};

    my @sortedTypes = SortTypes(\@typeObjects);
    
    # Create type variables declarations
    for my $type ( @sortedTypes ) {
        my $objectName = $type->{NAME};
        DumpTypeDecl($objectName, $type);
    }
    
    # Create initialization code
    for my $type ( @sortedTypes ) {
        my $objectName = $type->{NAME};
        DumpTypeInit($objectName, $type);
    }

    print "\n";
}

sub DumpEventMapSwitchStatement {
    my $defaultEventName = shift;
    my $ignoreCase = shift;
    my $mixedCaseExists = 0;

    print "\t\t\tswitch (type) {\n";
    for my $interfaceName (sort { "\U$a" cmp "\U$b" } keys %{$parseResult->{INTERFACES}} ) {
        my $interface = $parseResult->{INTERFACES}{$interfaceName};
        if (defined $interface->{ATTRIBUTES}->{MSRELATEDEVENT} && $interfaceName ne $defaultEventName) {
            for my $eventName (sort { "\U$a" cmp "\U$b" } keys %{$interface->{ATTRIBUTES}->{MSRELATEDEVENT}}) {
                if(lc($eventName) ne $eventName) {
                    $mixedCaseExists = 1;
                }
                if ($ignoreCase == 1) {
                    print "\t\t\t\tcase '" . lc($eventName) . "':\n";
                }
                else {
                    print "\t\t\t\tcase '$eventName':\n";
                }
            }
            print "\t\t\t\t\treturn $interfaceName;\n";
        }
    }
    print "\t\t\t};\n";
    return $mixedCaseExists;
}

sub DumpTagNameMapSwitchStatement {
    my $defaultElementName = 'HTMLElement';
    my $tagList = {};
    $tagList->{'unknown'} = 'HTMLUnknownElement';
    
    for my $interfaceName (sort { "\U$a" cmp "\U$b" } keys %{$parseResult->{INTERFACES}} ) {
        my $interface = $parseResult->{INTERFACES}{$interfaceName};
        if (defined $interface->{ATTRIBUTES}->{MSRELATEDELEMENT} && $interfaceName ne $defaultElementName) {
            for my $tagName (sort { "\U$a" cmp "\U$b" } keys %{$interface->{ATTRIBUTES}->{MSRELATEDELEMENT}}) {
                if (!defined $tagList->{$tagName}) { 
                    $tagList->{$tagName} = $interfaceName;
                }
            }
        }
    }

    print "\t\tswitch (tagName.toLowerCase()) {\n";
    for my $key (sort { "\U$a" cmp "\U$b" } keys %{$tagList} ) {
            print "\t\t\tcase '" . lc($key) . "' : return " . $tagList->{$key} . ";\n";
    }
    print "\t\t\tdefault: return HTMLElement;\n";
    print "\t\t};\n";
}

if ($flavor eq "worker") {
    # source of missing types %SDXROOT%\inetcore\mshtml\src\site\base\workerscriptholder.cxx
    my @missingTypes = (
        "Event",
        "MessageEvent",
        "MessagePort",
        "MessageChannel",
        "Worker",
        "IDBFactory",
        "IDBDatabase",
        "IDBObjectStore",
        "IDBIndex",
        "IDBCursor",
        "IDBCursorWithValue",
        "IDBKeyRange",
        "IDBTransaction",
        "IDBRequest",
        "IDBOpenDBRequest",
        "EventException",
        "DOMException",
        "ErrorEvent",
        "WebSocket",
        "CloseEvent",
        "XMLHttpRequest",
        "MSStream",
        "Blob",
        "MSBlobBuilder",
        "ImageData",
        "File",
        "FileList",
        "FileReader",
        "ProgressEvent",
        "MSStreamReader",
        "MSApp",
        "Console",
        "DOMStringList",
        "IDBVersionChangeEvent",
        "DOMError"
        );
    for my $type (@missingTypes) {
        CopyInterface($type, $additionalDefintions, $parseResult);
    }

    EnsureClosure();
}

if ($flavor eq "web" || $flavor eq "windows") {
    print <<EOF;
var document = { };
EOF
}

print <<EOF;
(function () {
    var _eventManager = _\$createEventManager(
    function getEventObject(type, attach, obj, ignoreCase) {
        function _eventTypeToObject(type, attach) {
            if(attach) return $defaultEventName;
EOF

my $mixedCaseEventNames = 0;
$mixedCaseEventNames = DumpEventMapSwitchStatement($defaultEventName, 0);

print <<EOF;
            return $defaultEventName;
        }
EOF
if ($mixedCaseEventNames != 0) {
    print <<EOF;
        function _eventTypeToObjectIgnoreCase(type, attach) {
            if(attach) return $defaultEventName;
            type = type.toLowerCase();
EOF

    DumpEventMapSwitchStatement($defaultEventName, 1);

    print <<EOF;
            return $defaultEventName;
        }
        var e = ignoreCase ? _eventTypeToObjectIgnoreCase(type, attach) : _eventTypeToObject(type, attach);
EOF
}
else {
    print <<EOF;
        var e = _eventTypeToObject(type, attach);
EOF
}
print <<EOF;
        var eventObject = Object.create(e);
        eventObject.target = obj;
        eventObject.currentTarget = obj;
        eventObject.type = type;
        if (eventObject.relatedTarget)
            eventObject.relatedTarget = obj;
        return eventObject;
    });
    var _events = _eventManager.createEventProperties;

EOF

# source: http://msdn.microsoft.com/en-us/library/ff975304(VS.85).aspx
if ($flavor eq "web" || $flavor eq "windows") {
    print <<EOF;
    function _createEvent(eventType) {
        function _eventTypeToObject(eventType) {
            if (eventType && typeof eventType === 'string') {
                switch(eventType.toLowerCase()) {
                    case 'compositionevent': return CompositionEvent;
                    case 'customevent': return CustomEvent;
                    case 'dragevent': return DragEvent;
                    case 'event':
                    case 'events': return Event;
                    case 'focusevent': return FocusEvent;
                    case 'keyboardevent': return KeyboardEvent;
                    case 'messageevent': return MessageEvent;
                    case 'mouseevent':
                    case 'mouseevents': return MouseEvent;
                    case 'mousewheelevent': return MouseWheelEvent;
                    case 'mutationevent':
                    case 'mutationevents': return MutationEvent;
                    case 'storageevent': return StorageEvent;
                    case 'svgzoomevents': return SVGZoomEvent;
                    case 'textevent': return TextEvent;
                    case 'uievent': 
                    case 'uievents': return UIEvent;
                    case 'wheelevent': return WheelEvent;
EOF
    if ($flavor eq "windows") {
        print <<EOF;
                    case 'errorevent': return ErrorEvent;
                    case 'animationevent': return AnimationEvent;
                    case 'msgestureevent': return MSGestureEvent;
                    case 'mspointerevent': return MSPointerEvent;
                    case 'transitionevent': return TransitionEvent;
                    case 'progressevent': return ProgressEvent;
EOF
    }

    print <<EOF;
                }
            }
        };
        var e = _eventTypeToObject(eventType);
        if(!e) e = Event;
        return Object.create(e);
    }

    function _getElementByTagName(tagName) {
        if (typeof tagName !== 'string') return;
EOF
    DumpTagNameMapSwitchStatement();
    print <<EOF;
    }

    function _getNewElementByTagName(tagName) {
        if (typeof tagName !== 'string') return;
        var element = Object.create(_getElementByTagName(tagName));
        element.localName = tagName;
        element.tagName = element.nodeName = tagName.toUpperCase(); 
        return element;
    }

    function _createDomObject(name) {
        return Window[name] && Window[name].prototype && Object.create(Window[name].prototype);
    };

    function _isAsyncScript(object) {
        return object && HTMLScriptElement.isPrototypeOf(object);
    }

    function _createElementByTagName(tagName) {
        if (typeof tagName !== 'string') return;
        var element = _getNewElementByTagName(tagName);
        element._\$searchable = true;
        return element;
    }

    function _wrapInList(list, resultListType, missingValueType, outputList) {
        var nodeList = typeof outputList !== 'undefined' ? outputList : Object.create(resultListType);
        var originalOutputListLength = typeof outputList !== 'undefined' ? outputList.length : 0;
        if (list) {
            for (var i = 0; i< list.length; i++) {
                nodeList[i] = list[i];
            }
            // clear any remaining items in outputList
            for (var i = list.length; i< originalOutputListLength; i++) {
                nodeList[i] = undefined;
            }
            nodeList.length = list.length;
        }
        if (missingValueType && nodeList.length == 0)
            nodeList[0] = _\$getTrackingUndefined(missingValueType);
        return nodeList;
    }

    function _createHTMLCollection(elementType) {
        var result = Object.create(HTMLCollection);
        result[0] = _\$getTrackingNull(_createElementByTagName(elementType));
        return result;
    }

    var _defaultScripts = [];

    function _scriptInDefaultList(scriptElement) {
        var found = false;
        if (scriptElement && scriptElement.src && _defaultScripts && _defaultScripts.length > 0) {
            _defaultScripts.forEach(function (entry) {
                if (scriptElement.src == entry.src)
                    found = true;
            });
        }
        return found;
    }

    function _getElementsByTagName(source, tagName) {
        var result = [];
        if (typeof tagName === 'string') {
            tagName = tagName.toLowerCase();
            if (source && source._\$searchable)
                return _findElementsByTagName(source, tagName);
            else if (tagName === 'script') {
                if (_defaultScripts.length > 0)
                    result = _\$asyncRequests.getItems().length == 1 ? _defaultScripts : _defaultScripts.concat(_\$asyncRequests.getItems());
                else
                    result = _\$asyncRequests.getItems();
            }
            else
                result = [ _getNewElementByTagName(tagName) ];
        }
        return _wrapInList(result, NodeList, _getNewElementByTagName(tagName));
    }

    function _findElementsByTagName(source, tagName, outputList) {
        var elements = [];
        _visitChildNodes(source, function(e) { 
            if (_isElement(e) && ('*' == tagName || e.tagName.toLowerCase() == tagName)) elements.push(e);		
        });
        var result = _wrapInList(elements, NodeList, _getNewElementByTagName(tagName), outputList);
        if (typeof outputList === 'undefined') {
            if (typeof source._\$queries === 'undefined')
                source._\$queries = [];
            source._\$queries.push({queryString: tagName, result: result});
        }
        return result;
    }

    function _visitChildNodes(start, callback) {
        if (_isNode(start) && _hasChildNodes(start)) {
                var q = [];
                q = q.concat(_childNodeList(start));
                var c = 0;
                while (q.length > 0 && c++ < 1000) {
                        var e = q.shift();
                        if (_isNode(e)) { 
                            callback(e);
                            if(_hasChildNodes(e)) q = q.concat(_childNodeList(e));
                        }
                }
        }
    }

    function _refreshQueries(node){
        if (_isNode(node)){
            if (node._\$queries)
                for(var i =0; i < node._\$queries.length; i++)
                    _findElementsByTagName(node, node._\$queries[i].queryString, node._\$queries[i].result) 
                // referesh the parent queries
                _refreshQueries(node.parentNode);
        }
    }

    function _embedAsyncRequest(originalObject, asyncRequest) {
        if (originalObject) {
            var newObject = Object.create(originalObject);
            _\$defineProperty(newObject, '_\$asyncRequest', asyncRequest);
            return newObject;
        }
        return originalObject;
    }

    function _getEmbeddedAsyncRequest(obj) {
        return (obj && obj._\$asyncRequest) ? obj._\$asyncRequest : obj;
    }

    function _isNode(n) {
        return typeof n !== 'undefined' && n && Node.isPrototypeOf(n);
    }

    function _isElement(e) {
        return typeof e !== 'undefined' && e && Element.isPrototypeOf(e);
    }

    function _getMatchingNull(obj) {
        return _\$getTrackingNull(Object.create(_isElement(obj) ? HTMLElement : Node));
    }

    function _isParentOf(parent, obj) {
        if (obj) {
            var cur = obj.parentNode;
            while (cur) {
                if (cur == parent) 
                    return true;
                cur = cur.parentNode;
            }
        }
        return false;
    }

    function _childNodes(obj, resultListType) {
        if (typeof obj._\$children === 'undefined')
            obj._\$children = Object.create(resultListType);
        return obj._\$children;
    }

    function _childNodeList(obj) {
        return typeof obj._\$children !== 'undefined'? Array.prototype.slice.call(obj._\$children) : [];
    }

    function _hasChildNodes(obj) {
        return typeof obj._\$children !== 'undefined' && obj._\$children.length > 0;
    }

    function _firstChild(obj, defaultObj) {
        return _hasChildNodes(obj) ? obj._\$children[0] : _\$getTrackingNull(Object.create(_isElement(obj) ? HTMLElement : defaultObj));
    }

    function _lastChild(obj, defaultObj) {
        return _hasChildNodes(obj) ? obj._\$children[obj._\$children.length - 1] : _\$getTrackingNull(Object.create(_isElement(obj) ? HTMLElement : defaultObj));
    }

    function _clearElement(obj) {
        if (_hasChildNodes(obj)) {
            for (var i = 0; i < obj._\$children.length; i++)
                obj._\$children[i].parentNode = obj._\$children[i].nextSibling = obj._\$children[i].previousSibling = _getMatchingNull(obj._\$children[i]);
            obj._\$children = undefined;
            _refreshQueries(obj);
        }
    }

    function _removeChild(obj, oldChild) {
        if (_isNode(oldChild) && _hasChildNodes(obj)) {
            for (var i = 0; i < obj._\$children.length; i++) {
                if (oldChild == obj._\$children[i]) {
                    if (oldChild.previousSibling) {
                        oldChild.previousSibling.nextSibling = oldChild.nextSibling;
                    }
                    if (oldChild.nextSibling) {
                        oldChild.nextSibling.previousSibling = oldChild.previousSibling;
                    }
                    Array.prototype.splice.call(obj._\$children, i, 1);
                    oldChild.parentNode = oldChild.nextSibling = oldChild.previousSibling = _getMatchingNull(obj);
                    _refreshQueries(obj);
                    break;
                }
            }
        }
        return oldChild;
    }

    function _appendChildInternal(obj, newChild) {
        if (_isNode(newChild) && obj != newChild && !_isParentOf(newChild, obj)) {
            if (newChild.parentNode)
                _removeChild(newChild.parentNode, newChild);
            if (typeof obj._\$children === 'undefined')
                obj._\$children = Object.create(NodeList);
            var previousSibling = obj._\$children.length >= 1 ? obj._\$children[obj._\$children.length - 1] : null;
            Array.prototype.push.call(obj._\$children, newChild);
            newChild.parentNode = obj;
            if (previousSibling) {
                newChild.previousSibling = previousSibling;
                previousSibling.nextSibling = newChild;
            }
            _refreshQueries(obj);
        }
        return newChild;
    }

    function _appendChild(obj, newChild) {
        if (_isAsyncScript(newChild) && !_scriptInDefaultList(newChild))
            _\$asyncRequests.add(newChild);
        return _appendChildInternal(obj, newChild);
    }

    function _insertBefore(obj, newChild, refChild) {
        if (_isNode(newChild) && obj != newChild && !_isParentOf(newChild, obj)) {
            if (newChild.parentNode)
                _removeChild(newChild.parentNode, newChild);
            if (typeof obj._\$children === 'undefined')
                obj._\$children = Object.create(NodeList);
            var index = 0;
            var nextSibling = null;
            var previousSibling = null;
            for (index = 0; index < obj._\$children.length; index++) {
                if (refChild == obj._\$children[index]) {
                    nextSibling = refChild;
                    break;
                }
                previousSibling = obj._\$children[index];
            }
            Array.prototype.splice.call(obj._\$children, index, 0, newChild);
            newChild.parentNode = obj;
            if (nextSibling) {
                newChild.nextSibling = nextSibling;
                nextSibling.previousSibling = newChild;
            }
            if (previousSibling) {
                newChild.previousSibling = previousSibling;
                previousSibling.nextSibling = newChild;
            }
            _refreshQueries(obj);
        }
        if (_isAsyncScript(newChild) && !_scriptInDefaultList(newChild))
            _\$asyncRequests.insertBefore(newChild, _getEmbeddedAsyncRequest(refChild));
        return newChild;
    }

    function _replaceChild(obj, newChild, oldChild) {
        if (_isNode(newChild) && obj != newChild && !_isParentOf(newChild, obj) && _isNode(oldChild) && _hasChildNodes(obj)) {
            for (var i = 0; i < obj._\$children.length; i++) {
                if (oldChild == obj._\$children[i]) {
                    if (newChild.parentNode)
                        _removeChild(newChild.parentNode, newChild);
                    newChild.previousSibling = oldChild.previousSibling;
                    newChild.nextSibling = oldChild.nextSibling;
                    if (oldChild.previousSibling) {
                        oldChild.previousSibling.nextSibling = newChild;
                    }
                    if (oldChild.nextSibling) {
                        oldChild.nextSibling.previousSibling = newChild;
                    }
                    newChild.parentNode = obj;
                    obj._\$children[i] = newChild;
                    oldChild.parentNode =	oldChild.nextSibling = oldChild.previousSibling = _getMatchingNull(obj);
                    _refreshQueries(obj);
                    break;
                }
            }
        }
        if (_isAsyncScript(newChild) && !_scriptInDefaultList(newChild))
            _\$asyncRequests.replace(newChild, _getEmbeddedAsyncRequest(oldChild));
        return oldChild;
    }

    function _firstElementChild(obj) {
        if (_isNode(obj)) {
            var cur = _firstChild(obj);
            do {
                if (_isElement(cur))
                    return cur;
                cur = cur.nextSibling;
            } while (cur);
        }
        return _\$getTrackingNull(Object.create(HTMLElement));
    }

    function _lastElementChild(obj) {
        if (_isNode(obj)) {
            var cur = _lastChild(obj);
            do {
                if (_isElement(cur))
                    return cur;
                cur = cur.previousSibling;
            } while (cur);
        }
        return _\$getTrackingNull(Object.create(HTMLElement));
    }

    function _nextElementSibling(obj) {
        if (_isNode(obj)) {
            var cur = obj.nextSibling;
            do {
                if (_isElement(cur))
                    return cur;
                cur = cur.nextSibling;
            } while (cur);
        }
        return _\$getTrackingNull(Object.create(HTMLElement));
    }

    function _previousElementSibling(obj) {
        if (_isNode(obj)) {
            var cur = obj.previousSibling;
            do {
                if (_isElement(cur))
                    return cur;
                cur = cur.previousSibling;
            } while (cur);
        }
        return _\$getTrackingNull(Object.create(HTMLElement));
    }

    function _parentElement(obj) {
        if (_isNode(obj)) {
            var cur = obj.parentNode;
            do {
                if (_isElement(cur))
                    return cur;
                cur = cur.parentNode;
            } while (cur);
        }
        return _\$getTrackingNull(Object.create(HTMLElement));
    }

    function _childElementCount(obj) {
        var count = 0;
        if (_isNode(obj)) {
            var cur = _firstChild(obj);
            do {
                if (_isElement(cur))
                    count ++;
                cur = cur.nextSibling;
            } while (cur);
        }
        return count;
    }

    function _applyElement(obj, apply, where) {
        if(!obj || !apply) return;
        if(where == undefined || where == "outside") {
            if(!obj.parentNode) return;
            _replaceChild(obj.parentNode, apply, obj);
            _appendChild(apply, obj);
        } 
        else if(where == 'inside') {
            var children = obj._\$children != undefined ? Array.prototype.slice.call(obj._\$children) : [];
            for(var i=0; i<children.length; i++) {
                _removeChild(obj, children[i]);
            }
            _appendChild(obj, apply);
            for(var i=0; i<children.length; i++) {
                _appendChild(apply, children[i]);
            }
        }
    }

    function _hasAttribute(object, name) {
        if (object)
            return object.hasOwnProperty(name);
        else
            return false;
    }

    function _setAttribute(object, name, value) {
        if (object)
            object[name] = value;
    }

    function _getAttribute(object, name) {
        if (_hasAttribute(object, name))
            return object[name];
        else 
            return null;
    }

    function _recordElementId(id, e) {
        if (typeof document._\$documentElements != 'object')
            document._\$documentElements = {};
        document._\$documentElements[id] = e;
    }

    function _lookupElement(id) {
        return (document._\$documentElements && document._\$documentElements[id]);
    }

    function _getElementById(elementId) {
        var element = _lookupElement(elementId) || Object.create(HTMLElement);
        element.id = elementId;
        return element;
    }

    var scriptTagRegEx = /<[\\s]*script[^>]*src[\\s]*=[\\s]*['"]([^'">]+)['"]/gim;
    function _setInnerHTML(source, content) {
            // since we are not parsing the inner html, mark the node as unsearchable
            source._\$searchable = false;
            var scriptTag = null;
            while (scriptTag = scriptTagRegEx.exec(content)) {
                    var scriptElement = Object.create(HTMLScriptElement);
                    scriptElement.src = scriptTag[1];
                    if (!_scriptInDefaultList(scriptElement))
                        _\$asyncRequests.add(scriptElement);
            }
    }

    function _formElements(form) {
        var elements = [];
        _visitChildNodes(form, function(node) {
            if(_isElement(node)) {
                var tagName = node.tagName.toLowerCase();
                if(tagName == 'input' || tagName == 'select' || tagName == 'button' || tagName == 'textarea' || tagName == 'fieldset') elements.push(node);
            }
        });
        return _wrapInList(elements, HTMLCollection, Object.create(HTMLElement));
    }

    function _selectOptions(select) {
        var options = [];
        _visitChildNodes(select, function(node) {
            var tagName = node.tagName.toLowerCase();
            if(tagName == 'option') options.push(node); 
            else if(tagName != 'optgroup') return false; 
        });
        return _wrapInList(options, HTMLCollection, _createElementByTagName('option'));
    }

    var queryIdSelectorRegEx = /^\\s*#([^<>\\s]+)\\s*\$/;
    function _queryIdSelector(selectors, returnFirstElementOnly) {
        var results = [];
        if (typeof selectors === 'string') {
            var parts = selectors.split(',');
            for (var i = 0; i < parts.length; i++) {
                var m = queryIdSelectorRegEx.exec(parts[i]);
                if (m && m[1]) {
                    var e = _lookupElement(m[1]);
                    if (e) {
                        if (returnFirstElementOnly) return e;
                        results.push(e);
                    }
                }
            }
        }
        if (!returnFirstElementOnly)
            return results;
    }

    function _querySelectorAll(obj, selectors) {
        var results = _queryIdSelector(selectors);
        if (results.length == 0)
            results = [Object.create(_getElementByTagName(selectors) || HTMLElement)];
        return _wrapInList(results, NodeList);
    }

    function _querySelector(obj, selectors) {
        var results = _queryIdSelector(selectors, true);
        if (!result)
            result = _\$getTrackingNull(Object.create(_getElementByTagName(selectors) || HTMLElement));
        return results;
    }

    function _extend(obj, original, filter) {
        if (obj && original) {
            var propertyNames = Object.getOwnPropertyNames(original);
            if (propertyNames && propertyNames.length > 0) {
                for (var p in propertyNames) {
                    var name = propertyNames[p];
                    if (typeof name != 'string' || (filter && name.match(filter))) continue;
                    Object.defineProperty(obj, name, Object.getOwnPropertyDescriptor(original, name));
                }
            }
        }
    }


    function _getConstructorFromString(type) {
        if (!typeof type === "string") {
            return;
        }

        var typeParts = type.split(".");
        var ctor = _\$globalObject;
        var i;
        for (i = 0; i < typeParts.length && ctor; i++) {
            ctor = ctor[typeParts[i]];
        }

        if (typeof ctor === "function") {
            return ctor;
        }
    }

    function _recordChildren(parent, elementDefinitions, parentForm) {
        if (_isElement(parent) && elementDefinitions && elementDefinitions.length > 0) {
            for (var i = 0 ; i < elementDefinitions.length; i++) {
                var e = elementDefinitions[i];
                if (e) {
                    var element = _createElementByTagName(e.\$tag);

                    // Insert in global lists
                    if (typeof e.id == 'string') {
                        _recordElementId(e.id, element);
                        // Simulate IE behaviour by exposing the element on the parent using its id
                        if (parentForm && e.\$formElement)
                            parentForm[e.id] = element;
                        else 
                            window[e.id] = element;
                    }

                    if (_isAsyncScript(element))
                        _defaultScripts.push(element);

                    // Initialize children
                    if (e.\$children)
                        _recordChildren(element, e.\$children, e.\$tag.toLowerCase() == 'form' ? element : parentForm);

                    // Copy properties
                    _extend(element, e, /(^[\\\$].+)|(^_\\\$fieldDoc\\\$\\\$.+)/);

                    if (e.\$object) {
                        _extend(element, e.\$object);
                    }

                    // Add winControl property if there is a data-win-control attribute
                    if (typeof e["data-win-control"] === "string") {
                        var winControlType = e["data-win-control"];
                        element.winControl = _\$initVar(undefined, {
                            ctor: _getConstructorFromString(winControlType),
                            type: winControlType,
                            isUnsafeType: true
                        });
                    }

                    _appendChildInternal(parent, element);
                }
            }
        }
    }

    function _recordDomStructure(elementDefinitions) {
        if (elementDefinitions && elementDefinitions.length > 0) {
            _clearElement(document.body);
            _clearElement(document.head);
            _defaultScripts = [];

            for (var i = 0 ; i < elementDefinitions.length; i++) {
                var e = elementDefinitions[i];
                if (e && e.\$tag && e.\$children) {
                    if (e.\$tag == 'body')
                        _recordChildren(document.body, e.\$children);
                    else if (e.\$tag == 'head')
                        _recordChildren(document.head, e.\$children);
                }
            }
        }
    }

    function _createIDBRequest(requestType, source, result){
        var request = Object.create(requestType);
        request.source = source;
        request.result = result;
        return request; 
    }


EOF
}

DumpTypes();

# Assign variables to emulate browser host
if ($flavor eq "web" || $flavor eq "windows") { 
    print <<EOF;

    // Assign variables to emulate browser host
    Document._\$createDomObject = _createDomObject;
    Document._\$recordDomStructure = _recordDomStructure;
    this.window = Window;
    _\$nonRemovable(this.window);
    document = Document;
    _publicObject('document', Document);
    document.nodeName = '#document';
    document.localName = _\$getTrackingNull('');
    document.nodeType = Node.DOCUMENT_NODE;
    document.documentMode = document.DOCUMENT_NODE;
    document.ownerDocument = _\$getTrackingNull(document);
    document.parentNode = _\$getTrackingNull(document);
    document.parentWindow = window;
    document.previousSibling = _\$getTrackingNull(document);
    document.nextSibling = _\$getTrackingNull(document);
    document.nodeValue = _\$getTrackingNull('');
    document.defaultView = window;

    document.head = _createElementByTagName('head');
    document.body = document.activeElement = _createElementByTagName('body');
    document.documentElement = _createElementByTagName('html');
    _appendChildInternal(document.documentElement, document.head);
    _appendChildInternal(document.documentElement, document.body);
    _appendChildInternal(document, document.documentElement);
    _appendChildInternal(document.head, _createElementByTagName('title'));
    _appendChildInternal(document.head, _createElementByTagName('script'));

    window.navigator.userAgent = 'Mozilla/5.0 (compatible; MSIE 9.0; Windows NT 6.1; WOW64; Trident/5.0; SLCC2; .NET CLR 2.0.50727; .NET CLR 3.5.30729; .NET CLR 3.0.30729; .NET4.0C; .NET4.0E; MS-RTC LM 8; InfoPath.3; Override:IE9_DEFAULT_20091014';
    window.location.href = 'about:blank';
    window.location.pathname = '/blank';
    window.location.protocol = 'about:';
    window.location.toString = function() { return this.href; }

    /* Wire all elements to have the body as their parent node */
    Node.parentNode = document.body;
    Node.ownerDocument = document;
EOF
}

# Public interface support
print <<EOF;

    function _publicInterface(name, interface, interfacePrototype) {
        _\$nonRemovable(interface);
        $globalObjectName\[name] = interface;
        $globalObjectName\[name].prototype = interfacePrototype;
    }

    function _publicObject(name, obj) {
        _\$nonRemovable(obj);
        $globalObjectName\[name] = obj;
    }
    
EOF

DumpPublicInterfaces();

DumpConstructors();

# Named Constructors
if ($flavor eq "web" || $flavor eq "windows") { 
    print <<EOF;

    function HTMLOptionElementFactory (text, value, defaultSelected, selected) {
        /// <signature>
        /// <param name='text' type='String' optional='true' />
        /// <param name='value' type='String' optional='true' />
        /// <param name='defaultSelected' type='Boolean' optional='true' />
        /// <param name='selected' type='Boolean' optional='true' />
        /// </signature>
        return Object.create(HTMLOptionElement);
    }

    function HTMLImageElementFactory(width, height) {
        /// <signature>
        /// <param name='width' type='Number' optional='true' />
        /// <param name='height' type='Number' optional='true' />
        /// </signature>
        return Object.create(HTMLImageElement);
    }

    function HTMLAudioElementFactory(src) {
        /// <signature>
        /// <param name='src' type='String' optional='true' />
        /// </signature>
        return Object.create(HTMLAudioElement);
    }
    
    _publicInterface('Option', HTMLOptionElementFactory, HTMLOptionElement);
    _publicInterface('Image', HTMLImageElementFactory, HTMLImageElement);
    _publicInterface('Audio', HTMLAudioElementFactory, HTMLAudioElement);
    
    intellisense.annotate(window, {
        Worker: function() {
            /// <signature>
            /// <param name='stringUrl' type='String' />
            /// </signature>
        },
        MSCSSMatrix: function () {
            /// <signature>
            /// <param name='text' type='String' optional='true' />
            /// </signature>
        },
        WebSocket: function() {
            /// <signature>
            /// <param name='url' type='String' />
            /// <param name='protocols' type='String' optional='true' />
            /// </signature>
            /// <signature>
            /// <param name='url' type='String' />
            /// <param name='protocols' type='Array' elementType='String' optional='true' />
            /// </signature>
        }
    });	

EOF

    # Legacy constructors do not show 'create' method in the specidl file. 
    # As a workaround add the create method explicitlly for the legacy constructor objects.
    # TODO: Remove this workaround when Bug 381816 is fixed.
    print <<EOF;
    window['Option'].create = window['Option'];
    window['Image'].create = window['Image'];
    window['XDomainRequest'].create = window['XDomainRequest'];
    window['XMLHttpRequest'].create = window['XMLHttpRequest'];

EOF
}

if ($flavor eq "worker") {
    print <<EOF;
    this['XMLHttpRequest'].create = this['XMLHttpRequest'];
EOF
}

print <<EOF;
})();

function _\$getActiveXObject(className, location) {
    if ((/XMLHTTP/i).test(className))
        return new window.XMLHttpRequest();
}
EOF
