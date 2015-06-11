//
// Blue 246936: RemoteStackWalker frame chain hop issue
//

(function foo() {
    SCA.serialize(foo);
})();
