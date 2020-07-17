

import * as Util from "./Util.bs.js";
import * as Block from "bs-platform/lib/es6/block.js";
import * as Curry from "bs-platform/lib/es6/curry.js";
import * as Manager from "./Manager.bs.js";
import * as Pervasives from "bs-platform/lib/es6/pervasives.js";
import * as Tablecloth from "tablecloth-bucklescript/bucklescript/src/tablecloth.bs.js";
import * as Caml_option from "bs-platform/lib/es6/caml_option.js";
import * as Caml_exceptions from "bs-platform/lib/es6/caml_exceptions.js";
import * as Caml_builtin_exceptions from "bs-platform/lib/es6/caml_builtin_exceptions.js";

var NotImplemented = Caml_exceptions.create("List.NotImplemented");

function listRemove(fn, list) {
  var foundItem = {
    contents: false
  };
  var newList = Tablecloth.List.filter((function (item) {
          var res = Curry._1(fn, item);
          if (!res) {
            foundItem.contents = true;
          }
          return res;
        }), list);
  if (foundItem.contents) {
    return /* Ok */Block.__(0, [newList]);
  } else {
    return /* Error */Block.__(1, [/* NotFound */-296251313]);
  }
}

function listFind(fn, data) {
  try {
    return /* Ok */Block.__(0, [Tablecloth.List.find(fn, data)]);
  }
  catch (exn){
    if (exn === Caml_builtin_exceptions.not_found) {
      return /* Error */Block.__(1, [/* NotFound */-296251313]);
    } else {
      throw exn;
    }
  }
}

function __findExn(_index, fn, _lst) {
  while(true) {
    var lst = _lst;
    var index = _index;
    if (lst) {
      var h = lst[0];
      if (Curry._1(fn, h)) {
        return /* tuple */[
                index,
                h
              ];
      } else {
        _lst = lst[1];
        _index = 1 + index | 0;
        continue ;
      }
    } else {
      throw [
            Caml_builtin_exceptions.failure,
            "Not Found"
          ];
    }
  };
}

function __insertAtExn(index, item, _currentIndex, lst) {
  index >= Tablecloth.List.length(lst);
  return Pervasives.$at(lst, /* :: */[
              item,
              /* [] */0
            ]);
}

function find(x, lst) {
  try {
    return __findExn(0, x, lst);
  }
  catch (exn){
    return ;
  }
}

function Make(Config) {
  var string_of_operation = function (param) {
    switch (param.tag | 0) {
      case /* Append */0 :
          return "Append";
      case /* Prepend */1 :
          return "Prepend";
      case /* InsertAt */2 :
          return "InsertAt";
      case /* AddAfter */3 :
          return "AddAfter";
      case /* AddBefore */4 :
          return "AddBefore";
      case /* Remove */5 :
          return "Remove";
      case /* Replace */6 :
          return "Replace";
      case /* Update */7 :
          return "Update";
      
    }
  };
  var internalId = Util.idOfString("__internal__");
  var wrapper = {
    contents: /* [] */0
  };
  var getSnapshot = function (param) {
    return wrapper.contents;
  };
  var get = function (id) {
    return Tablecloth.Result.fromOption("Item not found", Tablecloth.List.find((function (item) {
                      return Curry._1(Config.getId, item) === id;
                    }), wrapper.contents));
  };
  var getExn = function (id) {
    return Tablecloth.$$Option.getExn(Tablecloth.Result.toOption(get(id)));
  };
  var setData = function (updatedInternalData) {
    wrapper.contents = updatedInternalData;
    return /* () */0;
  };
  var changeListeners = {
    contents: /* [] */0
  };
  var addChangeListener = function (fn) {
    changeListeners.contents = /* :: */[
      fn,
      changeListeners.contents
    ];
    return /* () */0;
  };
  var removeChangeListener = function (fn) {
    changeListeners.contents = Tablecloth.List.filter((function (changeFn) {
            return changeFn !== fn;
          }), changeListeners.contents);
    return /* () */0;
  };
  var callChangeListeners = function (ops) {
    return Tablecloth.List.iter((function (fn) {
                  return Curry._1(fn, ops);
                }), changeListeners.contents);
  };
  var handleOperation = function (handleUndo, data, op) {
    switch (op.tag | 0) {
      case /* Append */0 :
          var t = op[0];
          Curry._1(handleUndo, /* Remove */Block.__(5, [Curry._1(Config.getId, t)]));
          return /* Ok */Block.__(0, [Pervasives.$at(data, /* :: */[
                          t,
                          /* [] */0
                        ])]);
      case /* Prepend */1 :
          var t$1 = op[0];
          Curry._1(handleUndo, /* Remove */Block.__(5, [Curry._1(Config.getId, t$1)]));
          return /* Ok */Block.__(0, [Pervasives.$at(/* :: */[
                          t$1,
                          /* [] */0
                        ], data)]);
      case /* InsertAt */2 :
          var t$2 = op[1];
          Curry._1(handleUndo, /* Remove */Block.__(5, [Curry._1(Config.getId, t$2)]));
          return /* Ok */Block.__(0, [Pervasives.$at(data, /* :: */[
                          t$2,
                          /* [] */0
                        ])]);
      case /* AddAfter */3 :
          var t$3 = op[1];
          var id = op[0];
          var newItemId = Curry._1(Config.getId, t$3);
          Curry._1(handleUndo, /* Remove */Block.__(5, [newItemId]));
          var foundItem = {
            contents: false
          };
          var res = Tablecloth.List.foldRight((function (item, newList) {
                  if (Curry._1(Config.getId, item) === id) {
                    foundItem.contents = true;
                    return /* :: */[
                            item,
                            /* :: */[
                              t$3,
                              newList
                            ]
                          ];
                  } else {
                    return /* :: */[
                            item,
                            newList
                          ];
                  }
                }), /* [] */0, data);
          if (foundItem.contents) {
            return /* Ok */Block.__(0, [res]);
          } else {
            return /* Error */Block.__(1, [/* NotFound */[op]]);
          }
      case /* AddBefore */4 :
          var t$4 = op[1];
          var id$1 = op[0];
          var newItemId$1 = Curry._1(Config.getId, t$4);
          Curry._1(handleUndo, /* Remove */Block.__(5, [newItemId$1]));
          var foundItem$1 = {
            contents: false
          };
          var res$1 = Tablecloth.List.foldRight((function (item, newList) {
                  if (Curry._1(Config.getId, item) === id$1) {
                    foundItem$1.contents = true;
                    return /* :: */[
                            t$4,
                            /* :: */[
                              item,
                              newList
                            ]
                          ];
                  } else {
                    return /* :: */[
                            item,
                            newList
                          ];
                  }
                }), /* [] */0, data);
          if (foundItem$1.contents) {
            return /* Ok */Block.__(0, [res$1]);
          } else {
            return /* Error */Block.__(1, [/* NotFound */[op]]);
          }
      case /* Remove */5 :
          var id$2 = op[0];
          var findRes = find((function (item) {
                  return Curry._1(Config.getId, item) === id$2;
                }), data);
          var match = listRemove((function (item) {
                  return Curry._1(Config.getId, item) !== id$2;
                }), data);
          if (findRes !== undefined) {
            var match$1 = findRes;
            if (match.tag) {
              return /* Error */Block.__(1, [/* NotFound */[op]]);
            } else {
              Curry._1(handleUndo, /* InsertAt */Block.__(2, [
                      match$1[0],
                      match$1[1]
                    ]));
              return match;
            }
          } else {
            return /* Error */Block.__(1, [/* NotFound */[op]]);
          }
      case /* Replace */6 :
          var t$5 = op[1];
          var id$3 = op[0];
          var match$2 = Tablecloth.List.find((function (item) {
                  return Curry._1(Config.getId, item) === id$3;
                }), data);
          if (match$2 !== undefined) {
            Curry._1(handleUndo, /* Replace */Block.__(6, [
                    id$3,
                    Caml_option.valFromOption(match$2)
                  ]));
            return /* Ok */Block.__(0, [Tablecloth.List.map((function (item) {
                              if (Curry._1(Config.getId, item) === id$3) {
                                return t$5;
                              } else {
                                return item;
                              }
                            }), data)]);
          } else {
            return /* Error */Block.__(1, [/* NotFound */[op]]);
          }
      case /* Update */7 :
          var update = op[1];
          var id$4 = op[0];
          var match$3 = Tablecloth.List.foldLeft((function (item, param) {
                  var l = param[0];
                  if (Curry._1(Config.getId, item) === id$4) {
                    var match = Curry._2(Config.reducer, item, update);
                    return /* tuple */[
                            /* :: */[
                              match[0],
                              l
                            ],
                            Caml_option.some(match[1])
                          ];
                  } else {
                    return /* tuple */[
                            /* :: */[
                              item,
                              l
                            ],
                            param[1]
                          ];
                  }
                }), /* tuple */[
                /* [] */0,
                undefined
              ], data);
          var undo = match$3[1];
          if (undo !== undefined) {
            Curry._1(handleUndo, /* Update */Block.__(7, [
                    id$4,
                    Caml_option.valFromOption(undo)
                  ]));
            return /* Ok */Block.__(0, [match$3[0]]);
          } else {
            return /* Error */Block.__(1, [/* NotFound */[op]]);
          }
      
    }
  };
  var baseApply = function (handleUndo, ops) {
    var param = Tablecloth.List.foldLeft((function (op, param) {
            var errors = param[1];
            var collection = param[0];
            var res = handleOperation(handleUndo, collection, op);
            if (res.tag) {
              return /* tuple */[
                      collection,
                      /* :: */[
                        res[0],
                        errors
                      ]
                    ];
            } else {
              return /* tuple */[
                      res[0],
                      errors
                    ];
            }
          }), /* tuple */[
          wrapper.contents,
          /* [] */0
        ], ops);
    var errors = param[1];
    wrapper.contents = param[0];
    if (errors) {
      return /* Error */Block.__(1, [errors]);
    } else {
      return /* Ok */Block.__(0, [/* () */0]);
    }
  };
  var applyRemoteOperations = function (param) {
    return baseApply((function (prim) {
                  return /* () */0;
                }), param);
  };
  Manager.register(Config.moduleId, baseApply);
  var apply = function (ops) {
    var res = Manager.apply(Config.moduleId, ops);
    callChangeListeners(ops);
    return res;
  };
  var addToTransaction = function (ops) {
    var prevCollection = wrapper.contents;
    return Manager.addToTransaction(Config.moduleId, ops, (function (param) {
                  wrapper.contents = prevCollection;
                  return /* () */0;
                }));
  };
  var length = function (param) {
    return Tablecloth.List.length(wrapper.contents);
  };
  var __resetCollection__ = function (param) {
    wrapper.contents = /* [] */0;
    changeListeners.contents = /* [] */0;
    return /* () */0;
  };
  return {
          string_of_operation: string_of_operation,
          internalId: internalId,
          wrapper: wrapper,
          getSnapshot: getSnapshot,
          get: get,
          getExn: getExn,
          setData: setData,
          changeListeners: changeListeners,
          addChangeListener: addChangeListener,
          removeChangeListener: removeChangeListener,
          callChangeListeners: callChangeListeners,
          handleOperation: handleOperation,
          baseApply: baseApply,
          applyRemoteOperations: applyRemoteOperations,
          apply: apply,
          addToTransaction: addToTransaction,
          length: length,
          __resetCollection__: __resetCollection__
        };
}

export {
  NotImplemented ,
  listRemove ,
  listFind ,
  __findExn ,
  __insertAtExn ,
  find ,
  Make ,
  
}
/* Manager Not a pure module */
