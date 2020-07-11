# credt

CRDT-ish data structures built for local-first, distributed applications.

## Why would I want this?

Credt makes it easier to build interactive, distributed apps.

- All data changes are applied atomically, which means most conflicts between different clients are avoided.
- In cases where there are conflicts, they are automatically resolved.
- Undo/redo is built-in.

## How does it work?

Credt is built on top of native data structures. But they don't have any functions that let's you operate on them. Instead, to modify your data you _apply operations_ on it. Operations are serializable, undoable and atomic.

#### Defining a data structure

Currently, this is what defining a credt list looks like:

```reason
module UserList = {
  type t = {
    id: Credt.Util.id,
    name: string,
    email: string,
    age: int,
  };

  type update =
    | SetEmail(string)
    | SetName(string)
    | SetAge(int);

  let reducer = user =>
    fun
    | SetEmail(email) => ({...user, email}, SetEmail(user.email))
    | SetName(name) => ({...user, name}, SetName(user.name))
    | SetAge(age) => ({...user, age}, SetAge(user.age));

  include Credt.List.Make({
    type nonrec t = t;
    type nonrec update = update;
    let getId = u => u.id;
    let moduleId = "UserList" |> Credt.Util.idOfString;
    let reducer = reducer;
  });
};
```

This might look like a lot – and there will be a ppx to remove the boilerplate – but it helps to understand what's going on inside credt. Let's go through it:

First we create the base type. This should be a record. The `id` field is required, and id:s are required to be unique. Credt has its own id type, and provides a function to generate id:s.

```reason
type t = {
  id: Credt.Util.id,
  name: string,
  email: string,
  age: int,
};
```

Then we define actions for the type, ie how the type can be modified. This will be used in the reducer later on.

```reason
type update =
  | SetEmail(string)
  | SetName(string)
  | SetAge(int);
```

Then the reducer, which takes a record of type `t` and an `update`. It returns a tuple `(t, update)` where `t` is a new object with the update applied, and `update` is the _undo_ update, which is used if the operation for some reason needs to be rolled back.

```reason
let reducer = user =>
  fun
  | SetEmail(email) => ({...user, email}, SetEmail(user.email))
  | SetName(name) => ({...user, name}, SetName(user.name))
  | SetAge(age) => ({...user, age}, SetAge(user.age));
```

Finally, we pass this into `Credt.List.Make` which is a functor that returns a Credt `List`. The `include` keyword means that everything defined in `Credt.List` will be included in our `UserList` module.

```reason
include Credt.List.Make({
  // Base (t) and update types.
  type nonrec t = t;
  type nonrec update = update;

  // A function to get the uniqueid from a record
  let getId = u => u.id;

  // A unique id for the module itself
  let moduleId = "UserList" |> Credt.Util.idOfString;

  // the reducer we defined above
  let reducer = reducer;
});
```

#### Modifying data

To make changes to the list we just defined, we apply operations on it. For example, adding a user looks like this:

```reason
let myUser = { ... };

let result = UserList.apply([Append(myUser)]);
```

To change the user's name:

```reason
let result = UserList.apply([Update(myUser.id, SetName("Maggie Simpson"))]);
```

`Append` and `Update` are variants that belong to `Credt.List`, and `SetName` is the variant we defined in our update type.

The result of an `apply` call is a `result(unit, list(failedOperations))`, so if some operations failed you can inform the user. This can happen if some other client had removed the record you tried to update for example, or if you yourself batched incompatible updates.

#### Reading data

`UserList.getSnapshot()` will return the current content of `UserList`, and `UserList.get(id)` will return a specific item. To use this in an app, you'd probably listen to updates and use `getSnapshot()` to pass data into your app.

### Undo & redo

Credt has global undo & redo functionality built it. Just call `Credt.Manager.undo()` to revert the latest operation. `Credt.Manager.redo()` will redo the last operation that was undone (if any).

## Developing:

```bash
npm install -g esy
git clone <this-repo>
esy install
esy build
```

## Running Tests:

```bash
# Runs the "test" command in `package.json`.
esy test
```

## Building

Credt is cross platform and compiles to native (with esy & dune) and javascript (with bucklescript).

```bash
esy x TestCredt.exe # Runs the native test build
yarn bsb -make-world -clean-world # Runs the bucklescript build
```

## Current status

Credt is under active development. Some parts are missing and the api is very likely to change.
