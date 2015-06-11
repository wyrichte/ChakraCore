function tag(strings, ...values) {
    _$trace(JSON.stringify(strings));
    _$trace(JSON.stringify(values/**getref:-2**/));
}

tag`foo${1}bar${2}`;

