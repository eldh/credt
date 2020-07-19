

import * as Block from "bs-platform/lib/es6/block.js";
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
    undoHistory.contents = Pervasives.$at(/* :: */[
          /* Op */Block.__(0, [/* tuple */[
                param[0],
                param[1]
              ]]),
          /* [] */0
        ], undoHistory.contents);
    return /* () */0;
  };
  var addRedo = function (param) {
    redoHistory.contents = Pervasives.$at(/* :: */[
          /* Op */Block.__(0, [/* tuple */[
                param[0],
                param[1]
              ]]),
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
  var applyTransaction = function (allowErrors, ops) {
    var undoOps = {
      contents: /* [] */0
    };
    var collectUndoOps = function (op) {
      undoOps.contents = Pervasives.$at(/* :: */[
            op,
            /* [] */0
          ], undoOps.contents);
      return /* () */0;
    };
    var match = Curry._2(C.apply, collectUndoOps, ops);
    if (match.tag) {
      if (allowErrors) {
        addUndoTransaction(undoOps.contents);
        undoOps.contents = /* [] */0;
        return match;
      } else {
        return match;
      }
    } else {
      addUndoTransaction(undoOps.contents);
      undoOps.contents = /* [] */0;
      return match;
    }
  };
  var canUndo = function (param) {
    return Tablecloth.List.length(undoHistory.contents) > 0;
  };
  var canRedo = function (param) {
    return Tablecloth.List.length(redoHistory.contents) > 0;
  };
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
        var match$2 = match$1[0];
        undoHistory.contents = match[1];
        return Curry._2(C.apply, addRedo, /* :: */[
                    /* tuple */[
                      match$2[0],
                      match$2[1]
                    ],
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
        var match$2 = match$1[0];
        redoHistory.contents = match[1];
        return Curry._2(C.apply, addUndo, /* :: */[
                    /* tuple */[
                      match$2[0],
                      match$2[1]
                    ],
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
  Make ,
  
}
/* Tablecloth Not a pure module */
