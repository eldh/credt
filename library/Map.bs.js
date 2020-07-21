

import * as Util from "./Util.bs.js";
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
      return {
              TAG: /* Ok */0,
              _0: Curry._2(IMap.remove, id, data)
            };
    } else {
      return {
              TAG: /* Error */1,
              _0: /* NotFound */-296251313
            };
    }
  };
  var mapFind = function (id, data) {
    var a = Curry._2(IMap.find_opt, id, data);
    if (a !== undefined) {
      return {
              TAG: /* Ok */0,
              _0: Caml_option.valFromOption(a)
            };
    } else {
      return {
              TAG: /* Error */1,
              _0: /* NotFound */-296251313
            };
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
      case /* Add */0 :
          var t = op._0;
          var id = Curry._1(Config.getId, t);
          Curry._1(handleUndo, {
                TAG: /* Remove */1,
                _0: id
              });
          return {
                  TAG: /* Ok */0,
                  _0: Curry._3(IMap.add, id, t, data)
                };
      case /* Remove */1 :
          var id$1 = op._0;
          var a = mapRemove(id$1, data);
          if (a.TAG) {
            return {
                    TAG: /* Error */1,
                    _0: /* NotFound */{
                      _0: op
                    }
                  };
          }
          Curry._1(handleUndo, {
                TAG: /* Add */0,
                _0: Curry._2(IMap.find, id$1, wrapper.contents)
              });
          return a;
      case /* Update */2 :
          var id$2 = op._0;
          var item = mapFind(id$2, data);
          if (item.TAG) {
            return {
                    TAG: /* Error */1,
                    _0: /* NotFound */{
                      _0: op
                    }
                  };
          }
          var match = Curry._2(Config.reducer, item._0, op._1);
          Curry._1(handleUndo, {
                TAG: /* Update */2,
                _0: id$2,
                _1: match[1]
              });
          return {
                  TAG: /* Ok */0,
                  _0: Curry._3(IMap.add, id$2, match[0], data)
                };
      
    }
  };
  var setMap = function (updatedInternalData) {
    wrapper.contents = updatedInternalData;
    
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
  Manager.register(Config.moduleId, baseApply);
  var applyRemoteOperations = function (param) {
    return baseApply((function (prim) {
                  
                }), param);
  };
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
  var __resetCollection__ = function (param) {
    wrapper.contents = IMap.empty;
    changeListeners.contents = /* [] */0;
    
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
          changeListeners: changeListeners,
          addChangeListener: addChangeListener,
          removeChangeListener: removeChangeListener,
          callChangeListeners: callChangeListeners,
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
