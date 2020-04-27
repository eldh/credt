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
    let print = u => u.name;
    let reducer = user =>
      fun
      | SetEmail(email) => ({...user, email}, SetEmail(user.email))
      | SetName(name) => ({...user, name}, SetName(user.name))
      | SetAge(age) => ({...user, age}, SetAge(user.age));
  });
};

module Post = {
  type t = {
    id: Rupp.Util.id,
    group: Rupp.Util.id,
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
    let print = u => u.content;
    let reducer = post =>
      fun
      | SetAuthor(author) => ({...post, author}, SetAuthor(post.author))
      | SetContent(content) => (
          {...post, content},
          SetContent(post.content),
        );
  });
};

module Group = {
  type t = {
    id: Rupp.Util.id,
    name: string,
    members: list(Rupp.Util.id),
  };

  type update =
    | SetName(string);

  include Rupp.Util.RMap({
    type nonrec t = t;
    type nonrec update = update;
    let print = u => u.name;
    let getId = u => u.id;
    let reducer = group =>
      fun
      | SetName(name) => ({...group, name}, SetName(group.name));
  });
};

[@ocaml.warning "-30"]
module Graph = {
  module Normalized = {
    type user = User.t;
    type group = Group.t;
    type post = Post.t;
    type update =
      | User(User.operation)
      | Group(Group.operation)
      | Post(Post.operation);

    let handleUpdate = update => {
      switch (update) {
      | User(operation) => User.save([operation])
      | Group(operation) => Group.save([operation])
      | Post(operation) => Post.save([operation])
      };
    };
    let save = updates => {
      List.iter(handleUpdate, updates);
    };
  };

  type post = {
    author: user,
    group,
    content: string,
    id: Rupp.Util.id,
  }
  and group = {
    name: string,
    members: list(groupUser),
    posts: list(post),
    id: Rupp.Util.id,
  }
  and user = {
    name: string,
    email: string,
    age: int,
    groups: list(group),
    posts: list(post),
    id: Rupp.Util.id,
  }
  and groupUser = {
    name: string,
    email: string,
    age: int,
    id: Rupp.Util.id,
  };

  let rec makeUser = (~users, ~groups, ~posts, user: User.t) => {
    id: user.id,
    email: user.email,
    name: user.name,
    age: user.age,
    posts:
      posts
      |> Post.toList
      |> List.filter((p: Post.t) => p.author === user.id)
      |> List.map(makePost(~users, ~posts, ~groups)),
    groups:
      groups
      |> Group.toList
      |> List.filter((group: Group.t) =>
           List.exists(id => id === user.id, group.members)
         )
      |> List.map(makeGroup(~users, ~groups, ~posts)),
  }
  and makeGroupUser = (~users, ~groups, ~posts, user: User.t) => {
    id: user.id,
    email: user.email,
    name: user.name,
    age: user.age,
  }
  and makeGroup = (~users, ~posts, ~groups, group: Group.t) => {
    id: group.id,
    name: group.name,
    members:
      group.members
      |> List.map(User.get)
      |> List.map(makeGroupUser(~users, ~groups, ~posts)),
    posts:
      posts
      |> Post.toList
      |> List.filter((p: Post.t) => p.group === group.id)
      |> List.map(makePost(~users, ~posts, ~groups)),
  }
  and makePost = (~users, ~groups, ~posts, post: Post.t) => {
    id: post.id,
    content: post.content,
    group: post.group |> Group.get |> makeGroup(~users, ~groups, ~posts),
    author: User.get(post.author) |> makeUser(~users, ~groups, ~posts),
  };

  type t = {
    posts: list(post),
    users: list(user),
    groups: list(group),
  };

  let make =
      (
        ~groups: Group.collection=Group.getCollection(),
        ~posts: Post.collection=Post.getCollection(),
        ~users: User.collection=User.getCollection(),
        (),
      ) => {
    {
      posts:
        posts |> Post.toList |> List.map(makePost(~users, ~groups, ~posts)),
      users:
        users |> User.toList |> List.map(makeUser(~users, ~groups, ~posts)),
      groups:
        groups
        |> Group.toList
        |> List.map(makeGroup(~users, ~groups, ~posts)),
    };
  };
};

describe("Set", ({test}) => {
  test("should update item", ({expect}) => {
    let me =
      User.{
        id: Rupp.Util.makeId(),
        name: "Andreas",
        email: "andreas@eldh.co",
        age: 35,
      };

    let miniMe =
      User.{
        id: Rupp.Util.makeId(),
        name: "Sixten",
        email: "sixten@eldh.co",
        age: 2,
      };
    let mom =
      User.{
        id: Rupp.Util.makeId(),
        name: "Linnéa",
        email: "linnea@wallen.co",
        age: 2,
      };
    let family =
      Group.{
        id: Rupp.Util.makeId(),
        name: "Eldhs",
        members: [miniMe.id, me.id, mom.id],
      };

    let updates =
      Graph.Normalized.[
        User(Add(me)),
        User(Update(me.id, SetEmail("a@eldh.co"))),
        User(Add(miniMe)),
        User(Add(mom)),
        Group(Add(family)),
        User(Update(miniMe.id, SetName("Sixten Eldh"))),
      ];

    expect.equal(me.email, "andreas@eldh.co");

    Graph.Normalized.save(updates);
    let me2 = User.get(me.id);
    let sixten2 = User.get(miniMe.id);
    let graph = Graph.make();
    graph.groups
    |> List.iter((g: Graph.group) => {
         print_endline("Group: " ++ g.name);
         g.members
         |> List.iter((g: Graph.groupUser) => {
              print_endline("Group user: " ++ g.name)
            });
       });
    expect.equal(me2.email, "a@eldh.co");
    expect.equal(sixten2.name, "Sixten Eldh");
    expect.equal(sixten2.age, 2);
  })
});
cli();