

import * as Curry from "bs-platform/lib/es6/curry.js";
import * as Pervasives from "bs-platform/lib/es6/pervasives.js";
import * as Tablecloth from "tablecloth-bucklescript/bucklescript/src/tablecloth.bs.js";

function Make(C) {
  var undoHistory = {
    contents: /* [] */0
  };
  var redoHistory = {
    contents: /* [] */0
  };
  var getUndoHistory = function (param) {
    return undoHistory.contents;
  };
  var getRedoHistory = function (param) {
    return redoHistory.contents;
  };
  var addUndo = function (param) {
    undoHistory.contents = Pervasives.$at({
          hd: {
            TAG: /* Op */0,
            _0: [
              param[0],
              param[1]
            ]
          },
          tl: /* [] */0
        }, undoHistory.contents);
    
  };
  var addRedo = function (param) {
    redoHistory.contents = Pervasives.$at({
          hd: {
            TAG: /* Op */0,
            _0: [
              param[0],
              param[1]
            ]
          },
          tl: /* [] */0
        }, redoHistory.contents);
    
  };
  var addUndoTransaction = function (ops) {
    undoHistory.contents = Pervasives.$at({
          hd: {
            TAG: /* Transaction */1,
            _0: ops
          },
          tl: /* [] */0
        }, undoHistory.contents);
    
  };
  var addRedoTransaction = function (ops) {
    redoHistory.contents = Pervasives.$at({
          hd: {
            TAG: /* Transaction */1,
            _0: ops
          },
          tl: /* [] */0
        }, redoHistory.contents);
    
  };
  var apply = Curry._1(C.apply, addUndo);
  var applyTransaction = function (allowErrors, ops) {
    var undoOps = {
      contents: /* [] */0
    };
    var collectUndoOps = function (op) {
      undoOps.contents = Pervasives.$at({
            hd: op,
            tl: /* [] */0
          }, undoOps.contents);
      
    };
    var match = Curry._2(C.apply, collectUndoOps, ops);
    if (match.TAG) {
      if (allowErrors) {
        addUndoTransaction(undoOps.contents);
        undoOps.contents = /* [] */0;
        return match;
      } else {
        return match;
      }
    }
    addUndoTransaction(undoOps.contents);
    undoOps.contents = /* [] */0;
    return match;
  };
  var canUndo = function (param) {
    return Tablecloth.List.length(undoHistory.contents) > 0;
  };
  var canRedo = function (param) {
    return Tablecloth.List.length(redoHistory.contents) > 0;
  };
  var undo = function (param) {
    var match = undoHistory.contents;
    if (!match) {
      return {
              TAG: /* Ok */0,
              _0: undefined
            };
    }
    var ops = match.hd;
    if (ops.TAG) {
      var ops$1 = ops._0;
      undoHistory.contents = match.tl;
      addRedoTransaction(ops$1);
      return Curry._2(C.apply, (function (prim) {
                    
                  }), ops$1);
    }
    var match$1 = ops._0;
    undoHistory.contents = match.tl;
    return Curry._2(C.apply, addRedo, {
                hd: [
                  match$1[0],
                  match$1[1]
                ],
                tl: /* [] */0
              });
  };
  var redo = function (param) {
    var match = redoHistory.contents;
    if (!match) {
      return {
              TAG: /* Ok */0,
              _0: undefined
            };
    }
    var ops = match.hd;
    if (ops.TAG) {
      var ops$1 = ops._0;
      redoHistory.contents = match.tl;
      addUndoTransaction(ops$1);
      return Curry._2(C.apply, (function (prim) {
                    
                  }), ops$1);
    }
    var match$1 = ops._0;
    redoHistory.contents = match.tl;
    return Curry._2(C.apply, addUndo, {
                hd: [
                  match$1[0],
                  match$1[1]
                ],
                tl: /* [] */0
              });
  };
  var __reset__ = function (param) {
    undoHistory.contents = /* [] */0;
    redoHistory.contents = /* [] */0;
    
  };
  return {
          undoHistory: undoHistory,
          redoHistory: redoHistory,
          getUndoHistory: getUndoHistory,
          getRedoHistory: getRedoHistory,
          addUndo: addUndo,
          addRedo: addRedo,
          addUndoTransaction: addUndoTransaction,
          addRedoTransaction: addRedoTransaction,
          apply: apply,
          applyTransaction: applyTransaction,
          canUndo: canUndo,
          canRedo: canRedo,
          undo: undo,
          redo: redo,
          __reset__: __reset__
        };
}

export {
  Make ,
  
}
/* Tablecloth Not a pure module */
