

import * as Util from "./Util.bs.js";
import * as Curry from "bs-platform/lib/es6/curry.js";
import * as Manager from "./Manager.bs.js";
import * as Pervasives from "bs-platform/lib/es6/pervasives.js";
import * as Tablecloth from "tablecloth/bucklescript/src/tablecloth.bs.js";
import * as Caml_option from "bs-platform/lib/es6/caml_option.js";
import * as Caml_exceptions from "bs-platform/lib/es6/caml_exceptions.js";
import * as Caml_js_exceptions from "bs-platform/lib/es6/caml_js_exceptions.js";

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
    return {
            TAG: /* Ok */0,
            _0: newList
          };
  } else {
    return {
            TAG: /* Error */1,
            _0: /* NotFound */-296251313
          };
  }
}

function listFind(fn, data) {
  try {
    return {
            TAG: /* Ok */0,
            _0: Tablecloth.List.find(fn, data)
          };
  }
  catch (raw_exn){
    var exn = Caml_js_exceptions.internalToOCamlException(raw_exn);
    if (exn.RE_EXN_ID === "Not_found") {
      return {
              TAG: /* Error */1,
              _0: /* NotFound */-296251313
            };
    }
    throw exn;
  }
}

function __findExn(_index, fn, _lst) {
  while(true) {
    var lst = _lst;
    var index = _index;
    if (lst) {
      var h = lst.hd;
      if (Curry._1(fn, h)) {
        return [
                index,
                h
              ];
      }
      _lst = lst.tl;
      _index = 1 + index | 0;
      continue ;
    }
    throw {
          RE_EXN_ID: "Failure",
          _1: "Not Found",
          Error: new Error()
        };
  };
}

function __insertAtExn(index, item, _currentIndex, lst) {
  index >= Tablecloth.List.length(lst);
  return Pervasives.$at(lst, {
              hd: item,
              tl: /* [] */0
            });
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
  var string_of_operation = function (_t) {
    switch (_t.TAG | 0) {
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
    
  };
  var changeListeners = {
    contents: /* [] */0
  };
  var addChangeListener = function (fn) {
    changeListeners.contents = {
      hd: fn,
      tl: changeListeners.contents
    };
    
  };
  var removeChangeListener = function (fn) {
    changeListeners.contents = Tablecloth.List.filter((function (changeFn) {
            return changeFn !== fn;
          }), changeListeners.contents);
    
  };
  var callChangeListeners = function (ops) {
    return Tablecloth.List.iter((function (fn) {
                  return Curry._1(fn, ops);
                }), changeListeners.contents);
  };
  var handleOperation = function (handleUndo, data, op) {
    switch (op.TAG | 0) {
      case /* Append */0 :
          var t = op._0;
          Curry._1(handleUndo, {
                TAG: /* Remove */5,
                _0: Curry._1(Config.getId, t)
              });
          return {
                  TAG: /* Ok */0,
                  _0: Pervasives.$at(data, {
                        hd: t,
                        tl: /* [] */0
                      })
                };
      case /* Prepend */1 :
          var t$1 = op._0;
          Curry._1(handleUndo, {
                TAG: /* Remove */5,
                _0: Curry._1(Config.getId, t$1)
              });
          return {
                  TAG: /* Ok */0,
                  _0: Pervasives.$at({
                        hd: t$1,
                        tl: /* [] */0
                      }, data)
                };
      case /* InsertAt */2 :
          var t$2 = op._1;
          Curry._1(handleUndo, {
                TAG: /* Remove */5,
                _0: Curry._1(Config.getId, t$2)
              });
          return {
                  TAG: /* Ok */0,
                  _0: Pervasives.$at(data, {
                        hd: t$2,
                        tl: /* [] */0
                      })
                };
      case /* AddAfter */3 :
          var t$3 = op._1;
          var id = op._0;
          var newItemId = Curry._1(Config.getId, t$3);
          Curry._1(handleUndo, {
                TAG: /* Remove */5,
                _0: newItemId
              });
          var foundItem = {
            contents: false
          };
          var res = Tablecloth.List.foldRight((function (item, newList) {
                  if (Curry._1(Config.getId, item) === id) {
                    foundItem.contents = true;
                    return {
                            hd: item,
                            tl: {
                              hd: t$3,
                              tl: newList
                            }
                          };
                  } else {
                    return {
                            hd: item,
                            tl: newList
                          };
                  }
                }), /* [] */0, data);
          if (foundItem.contents) {
            return {
                    TAG: /* Ok */0,
                    _0: res
                  };
          } else {
            return {
                    TAG: /* Error */1,
                    _0: /* NotFound */{
                      _0: op
                    }
                  };
          }
      case /* AddBefore */4 :
          var t$4 = op._1;
          var id$1 = op._0;
          var newItemId$1 = Curry._1(Config.getId, t$4);
          Curry._1(handleUndo, {
                TAG: /* Remove */5,
                _0: newItemId$1
              });
          var foundItem$1 = {
            contents: false
          };
          var res$1 = Tablecloth.List.foldRight((function (item, newList) {
                  if (Curry._1(Config.getId, item) === id$1) {
                    foundItem$1.contents = true;
                    return {
                            hd: t$4,
                            tl: {
                              hd: item,
                              tl: newList
                            }
                          };
                  } else {
                    return {
                            hd: item,
                            tl: newList
                          };
                  }
                }), /* [] */0, data);
          if (foundItem$1.contents) {
            return {
                    TAG: /* Ok */0,
                    _0: res$1
                  };
          } else {
            return {
                    TAG: /* Error */1,
                    _0: /* NotFound */{
                      _0: op
                    }
                  };
          }
      case /* Remove */5 :
          var id$2 = op._0;
          var findRes = find((function (item) {
                  return Curry._1(Config.getId, item) === id$2;
                }), data);
          var match = listRemove((function (item) {
                  return Curry._1(Config.getId, item) !== id$2;
                }), data);
          if (findRes === undefined) {
            return {
                    TAG: /* Error */1,
                    _0: /* NotFound */{
                      _0: op
                    }
                  };
          }
          if (match.TAG) {
            return {
                    TAG: /* Error */1,
                    _0: /* NotFound */{
                      _0: op
                    }
                  };
          }
          Curry._1(handleUndo, {
                TAG: /* InsertAt */2,
                _0: findRes[0],
                _1: findRes[1]
              });
          return match;
      case /* Replace */6 :
          var t$5 = op._1;
          var id$3 = op._0;
          var item = Tablecloth.List.find((function (item) {
                  return Curry._1(Config.getId, item) === id$3;
                }), data);
          if (item !== undefined) {
            Curry._1(handleUndo, {
                  TAG: /* Replace */6,
                  _0: id$3,
                  _1: Caml_option.valFromOption(item)
                });
            return {
                    TAG: /* Ok */0,
                    _0: Tablecloth.List.map((function (item) {
                            if (Curry._1(Config.getId, item) === id$3) {
                              return t$5;
                            } else {
                              return item;
                            }
                          }), data)
                  };
          } else {
            return {
                    TAG: /* Error */1,
                    _0: /* NotFound */{
                      _0: op
                    }
                  };
          }
      case /* Update */7 :
          var update = op._1;
          var id$4 = op._0;
          var match$1 = Tablecloth.List.foldLeft((function (item, param) {
                  var l = param[0];
                  if (Curry._1(Config.getId, item) !== id$4) {
                    return [
                            {
                              hd: item,
                              tl: l
                            },
                            param[1]
                          ];
                  }
                  var match = Curry._2(Config.reducer, item, update);
                  return [
                          {
                            hd: match[0],
                            tl: l
                          },
                          Caml_option.some(match[1])
                        ];
                }), [
                /* [] */0,
                undefined
              ], data);
          var undo = match$1[1];
          if (undo !== undefined) {
            Curry._1(handleUndo, {
                  TAG: /* Update */7,
                  _0: id$4,
                  _1: Caml_option.valFromOption(undo)
                });
            return {
                    TAG: /* Ok */0,
                    _0: match$1[0]
                  };
          } else {
            return {
                    TAG: /* Error */1,
                    _0: /* NotFound */{
                      _0: op
                    }
                  };
          }
      
    }
  };
  var baseApply = function (handleUndo, ops) {
    var param = Tablecloth.List.foldLeft((function (op, param) {
            var errors = param[1];
            var collection = param[0];
            var res = handleOperation(handleUndo, collection, op);
            if (res.TAG) {
              return [
                      collection,
                      {
                        hd: res._0,
                        tl: errors
                      }
                    ];
            } else {
              return [
                      res._0,
                      errors
                    ];
            }
          }), [
          wrapper.contents,
          /* [] */0
        ], ops);
    var errors = param[1];
    wrapper.contents = param[0];
    if (errors) {
      return {
              TAG: /* Error */1,
              _0: errors
            };
    } else {
      return {
              TAG: /* Ok */0,
              _0: undefined
            };
    }
  };
  var applyRemoteOperations = function (param) {
    return baseApply((function (prim) {
                  
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
                  
                }));
  };
  var length = function (param) {
    return Tablecloth.List.length(wrapper.contents);
  };
  var __resetCollection__ = function (param) {
    wrapper.contents = /* [] */0;
    changeListeners.contents = /* [] */0;
    
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
