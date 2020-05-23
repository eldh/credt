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

```
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
    let reducer = reducer;
  });
};
```

This might look like a lot – and there will be a ppx to remove the boilerplate – but it helps to understand what's going on inside crdt. Let's go through it:

First we create the base type. This should be a record. The `id` field is required, and id:s are required to be unique. Credt has its own id type, and provides a function to generate id:s.

```
type t = {
  id: Credt.Util.id,
  name: string,
  email: string,
  age: int,
};
```

Then we define actions for the type, ie how the type can be modified. This will be used in the reducer later on.

```
type update =
  | SetEmail(string)
  | SetName(string)
  | SetAge(int);
```

Then the reducer, which takes a record of type `t` and an `update`. It returns a tuple `(t, update)` where `t` is a new object with the update applied, and `update` is the _undo_ update.

```
let reducer = user =>
  fun
  | SetEmail(email) => ({...user, email}, SetEmail(user.email))
  | SetName(name) => ({...user, name}, SetName(user.name))
  | SetAge(age) => ({...user, age}, SetAge(user.age));
```

Finally, we pass this into `Credt.List.Make` which is a functor that returns a Credt `List`. The `include` keyword means that everything defined in `Credt.List` will be inclded in our `UserList` module

```
include Credt.List.Make({
  type nonrec t = t;
  type nonrec update = update;
  let getId = u => u.id;
  let reducer = reducer;
});
```

#### Modifying data

To make changes to the list we just defined, we apply operations on it. For example, adding a user looks like this

```
let myUser = { ... };

UserList.apply([Append(myUser)]);
```

To change the user's name:

```
let result = UserList.apply([Update(myUser.id, SetName("Maggie Simpson"))]);
```

`Append` and `Update` are variants that belong to `Credt.List`, and `SetName` is the variant we defined in our update type.

The result of an `apply` call is a `result(unit, list(failedOperations))`, so if some operations failed you can inform the user. This can happen if some other client had removed the record you tried to update for example.

## Developing:

```
npm install -g esy
git clone <this-repo>
esy install
esy build
```

## Running Binary:

After building the project, you can run the main binary that is produced.

```
esy x CredtApp.exe
```

## Running Tests:

```
# Runs the "test" command in `package.json`.
esy test
```
