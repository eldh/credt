

import * as Block from "bs-platform/lib/es6/block.js";
import * as Curry from "bs-platform/lib/es6/curry.js";
import * as Pervasives from "bs-platform/lib/es6/pervasives.js";
import * as Tablecloth from "tablecloth-bucklescript/bucklescript/src/tablecloth.bs.js";
import * as Caml_exceptions from "bs-platform/lib/es6/caml_exceptions.js";

var NotImplemented = Caml_exceptions.create("UndoRedo.NotImplemented");

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
  var addUndo = function (op) {
    undoHistory.contents = Pervasives.$at(/* :: */[
          /* Op */Block.__(0, [op]),
          /* [] */0
        ], undoHistory.contents);
    return /* () */0;
  };
  var addRedo = function (op) {
    redoHistory.contents = Pervasives.$at(/* :: */[
          /* Op */Block.__(0, [op]),
          /* [] */0
        ], redoHistory.contents);
    return /* () */0;
  };
  var addUndoTransaction = function (ops) {
    undoHistory.contents = Pervasives.$at(/* :: */[
          /* Transaction */Block.__(1, [ops]),
          /* [] */0
        ], undoHistory.contents);
    return /* () */0;
  };
  var addRedoTransaction = function (ops) {
    redoHistory.contents = Pervasives.$at(/* :: */[
          /* Transaction */Block.__(1, [ops]),
          /* [] */0
        ], redoHistory.contents);
    return /* () */0;
  };
  var apply = Curry._1(C.apply, addUndo);
  var applyTransaction = function (ops) {
    var ok = Curry._2(C.apply, (function (prim) {
            return /* () */0;
          }), ops);
    if (ok.tag) {
      return ok;
    } else {
      addUndoTransaction(ops);
      return ok;
    }
  };
  var canUndo = Tablecloth.List.length(undoHistory.contents) > 0;
  var canRedo = Tablecloth.List.length(redoHistory.contents) > 0;
  var undo = function (param) {
    var match = undoHistory.contents;
    if (match) {
      var match$1 = match[0];
      if (match$1.tag) {
        var ops = match$1[0];
        undoHistory.contents = match[1];
        addRedoTransaction(ops);
        return Curry._2(C.apply, (function (prim) {
                      return /* () */0;
                    }), ops);
      } else {
        undoHistory.contents = match[1];
        return Curry._2(C.apply, addRedo, /* :: */[
                    match$1[0],
                    /* [] */0
                  ]);
      }
    } else {
      return /* Ok */Block.__(0, [/* () */0]);
    }
  };
  var redo = function (param) {
    var match = redoHistory.contents;
    if (match) {
      var match$1 = match[0];
      if (match$1.tag) {
        var ops = match$1[0];
        redoHistory.contents = match[1];
        addUndoTransaction(ops);
        return Curry._2(C.apply, (function (prim) {
                      return /* () */0;
                    }), ops);
      } else {
        redoHistory.contents = match[1];
        return Curry._2(C.apply, addUndo, /* :: */[
                    match$1[0],
                    /* [] */0
                  ]);
      }
    } else {
      return /* Ok */Block.__(0, [/* () */0]);
    }
  };
  var __reset__ = function (param) {
    undoHistory.contents = /* [] */0;
    redoHistory.contents = /* [] */0;
    return /* () */0;
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
  NotImplemented ,
  Make ,
  
}
/* Tablecloth Not a pure module */
