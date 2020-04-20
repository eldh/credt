open TestFramework;
module User = {
  type t = {
    id: Rupp.Util.id,
    name: string,
    email: string,
    age: int,
  };

  type update =
    | SetEmail(string)
    | SetName(string)
    | SetAge(int);

  include Rupp.Util.RMap({
    type nonrec t = t;
    type nonrec update = update;
    let getId = u => u.id;
    let reducer = (user, update) =>
      switch (update) {
      | SetEmail(email) => ({...user, email}, SetEmail(user.email))
      | SetName(name) => ({...user, name}, SetName(user.name))
      | SetAge(age) => ({...user, age}, SetAge(user.age))
      };
  });
};

module Post = {
  type t = {
    id: Rupp.Util.id,
    author: Rupp.Util.id,
    content: string,
  };

  type update =
    | SetAuthor(Rupp.Util.id)
    | SetContent(string);

  include Rupp.Util.RMap({
    type nonrec t = t;
    type nonrec update = update;
    let getId = u => u.id;
    let reducer = (post, update) =>
      switch (update) {
      | SetAuthor(author) => ({...post, author}, SetAuthor(post.author))
      | SetContent(content) => (
          {...post, content},
          SetContent(post.content),
        )
      };
  });
};

[@ocaml.warning "-30"]
module Graph = {
  type post = {
    author: user,
    content: string,
    id: Rupp.Util.id,
  }
  and user = {
    name: string,
    email: string,
    age: int,
    posts: list(post),
    id: Rupp.Util.id,
  };

  let rec denormalizeUser = (posts, user: User.t) => {
    id: user.id,
    email: user.email,
    name: user.name,
    age: user.age,
    posts:
      posts
      |> Post.toList
      |> List.filter((p: Post.t) => p.author === user.id)
      |> List.map(denormalizePost),
  }
  and denormalizePost = (post: Post.t) => {
    id: post.id,
    content: post.content,
    author: User.get(post.author) |> denormalizeUser(Post.getMap_()),
  };
  type t = {
    posts: list(post),
    users: list(user),
  };

  let make = (~posts: Post.IMap.t(Post.t), ~users: User.IMap.t(User.t), ()) => {
    {
      posts: posts |> Post.toList |> List.map(denormalizePost),
      users: users |> User.toList |> List.map(denormalizeUser(posts)),
    };
  };
};

describe("Set", ({test}) => {
  test("should update item", ({expect}) => {
    let me =
      User.{
        id: "1" |> Rupp.Util.makeId,
        name: "Andreas",
        email: "andreas@eldh.co",
        age: 35,
      };

    let miniMe =
      User.{
        id: "2" |> Rupp.Util.makeId,
        name: "Sixten",
        email: "sixten@eldh.co",
        age: 2,
      };

    let updates =
      User.[
        add(me),
        update(me, SetEmail("a@eldh.co")),
        add(miniMe),
        remove(miniMe),
        update(miniMe, SetName("Sixten Eldh")),
      ];

    expect.equal(me.email, "andreas@eldh.co");

    User.save(updates);

    let me2 = User.get(me.id);
    let sixten2 = User.get(miniMe.id);

    expect.equal(me2.email, "a@eldh.co");
    expect.equal(sixten2.name, "Sixten Eldh");
    expect.equal(sixten2.age, 2);
  })
});
cli();