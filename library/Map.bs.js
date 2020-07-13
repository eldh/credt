

import * as Util from "./Util.bs.js";
import * as Block from "bs-platform/lib/es6/block.js";
import * as Curry from "bs-platform/lib/es6/curry.js";
import * as Stdlib from "../bs/Stdlib.bs.js";
import * as Manager from "./Manager.bs.js";
import * as Caml_obj from "bs-platform/lib/es6/caml_obj.js";
import * as Tablecloth from "tablecloth-bucklescript/bucklescript/src/tablecloth.bs.js";
import * as Caml_option from "bs-platform/lib/es6/caml_option.js";
import * as Caml_exceptions from "bs-platform/lib/es6/caml_exceptions.js";

var NotImplemented = Caml_exceptions.create("Map.NotImplemented");

function Make(Config) {
  var compare = Caml_obj.caml_compare;
  var IMap = Stdlib.$$Map.Make({
        compare: compare
      });
  var mapRemove = function (id, data) {
    var match = Curry._2(IMap.find_opt, id, data);
    if (match !== undefined) {
      return /* Ok */Block.__(0, [Curry._2(IMap.remove, id, data)]);
    } else {
      return /* Error */Block.__(1, [/* NotFound */-296251313]);
    }
  };
  var mapFind = function (id, data) {
    var match = Curry._2(IMap.find_opt, id, data);
    if (match !== undefined) {
      return /* Ok */Block.__(0, [Caml_option.valFromOption(match)]);
    } else {
      return /* Error */Block.__(1, [/* NotFound */-296251313]);
    }
  };
  var internalId = Util.idOfString("__internal__");
  var internalData = IMap.empty;
  var wrapper = {
    contents: internalData
  };
  var getSnapshot = function (param) {
    return wrapper.contents;
  };
  var get = function (id) {
    return Curry._2(IMap.find, id, wrapper.contents);
  };
  var handleOperation = function (handleUndo, data, op) {
    switch (op.tag | 0) {
      case /* Add */0 :
          var t = op[0];
          var id = Curry._1(Config.getId, t);
          Curry._1(handleUndo, /* Remove */Block.__(1, [id]));
          return /* Ok */Block.__(0, [Curry._3(IMap.add, id, t, data)]);
      case /* Remove */1 :
          var id$1 = op[0];
          var a = mapRemove(id$1, data);
          if (a.tag) {
            return /* Error */Block.__(1, [/* NotFound */[op]]);
          } else {
            Curry._1(handleUndo, /* Add */Block.__(0, [Curry._2(IMap.find, id$1, wrapper.contents)]));
            return a;
          }
      case /* Update */2 :
          var id$2 = op[0];
          var match = mapFind(id$2, data);
          if (match.tag) {
            return /* Error */Block.__(1, [/* NotFound */[op]]);
          } else {
            var match$1 = Curry._2(Config.reducer, match[0], op[1]);
            Curry._1(handleUndo, /* Update */Block.__(2, [
                    id$2,
                    match$1[1]
                  ]));
            return /* Ok */Block.__(0, [Curry._3(IMap.add, id$2, match$1[0], data)]);
          }
      
    }
  };
  var setMap = function (updatedInternalData) {
    wrapper.contents = updatedInternalData;
    return /* () */0;
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
  Manager.register(Config.moduleId, baseApply);
  var applyRemoteOperations = function (param) {
    return baseApply((function (prim) {
                  return /* () */0;
                }), param);
  };
  var apply = function (ops) {
    return Manager.apply(Config.moduleId, ops);
  };
  var addToTransaction = function (ops) {
    var prevCollection = wrapper.contents;
    return Manager.addToTransaction(Config.moduleId, ops, (function (param) {
                  wrapper.contents = prevCollection;
                  return /* () */0;
                }));
  };
  var __resetCollection__ = function (param) {
    wrapper.contents = IMap.empty;
    return /* () */0;
  };
  var toList = function (m) {
    return Tablecloth.List.map((function (param) {
                  return param[1];
                }), Curry._1(IMap.to_seq, m));
  };
  return {
          IMap: IMap,
          mapRemove: mapRemove,
          mapFind: mapFind,
          internalId: internalId,
          internalData: internalData,
          wrapper: wrapper,
          getSnapshot: getSnapshot,
          get: get,
          handleOperation: handleOperation,
          setMap: setMap,
          baseApply: baseApply,
          applyRemoteOperations: applyRemoteOperations,
          apply: apply,
          addToTransaction: addToTransaction,
          __resetCollection__: __resetCollection__,
          toList: toList
        };
}

export {
  NotImplemented ,
  Make ,
  
}
/* Manager Not a pure module */
