class BBC {
    foo() {
    }
}
var x2 = new BBC()/**getref:-3**/;

class Person {
    constructor() { }
    get firstName() {
        return this._firstName
    }
    set firstName(firstName) {
        this._firstName = firstName;
    }

    get lastName() {
        return this._lastName
    }

    set lastName(lastName) {
        this._lastName = lastName;
    }

}

var x = new Person()/**getref:-3**/;
x.firstName/**getref:-3**/;

