

import * as Curry from "bs-platform/lib/es6/curry.js";
import * as Tablecloth from "tablecloth-bucklescript/bucklescript/src/tablecloth.bs.js";
import * as Caml_option from "bs-platform/lib/es6/caml_option.js";
import * as UndoRedo$Credt from "./UndoRedo.bs.js";

function toOp(prim) {
  return prim;
}

var applyFns = {
  contents: /* [] */0
};

function register(id, apply) {
  applyFns.contents = {
    hd: [
      id,
      apply
    ],
    tl: applyFns.contents
  };
  
}

function mapError(fn, a) {
  if (a.TAG) {
    return {
            TAG: /* Error */1,
            _0: Curry._1(fn, a._0)
          };
  } else {
    return a;
  }
}

var changeListeners = {
  contents: /* [] */0
};

function addChangeListener(fn) {
  changeListeners.contents = {
    hd: fn,
    tl: changeListeners.contents
  };
  
}

function removeChangeListener(fn) {
  changeListeners.contents = Tablecloth.List.filter((function (changeFn) {
          return changeFn !== fn;
        }), changeListeners.contents);
  
}

function callChangeListeners(ops) {
  return Tablecloth.List.iter((function (fn) {
                return Curry._1(fn, ops);
              }), changeListeners.contents);
}

function baseApply(handleUndo, ops) {
  var res = Tablecloth.List.fold_left((function (res, memo) {
          if (memo.TAG) {
            if (res.TAG) {
              return {
                      TAG: /* Error */1,
                      _0: Tablecloth.List.concat({
                            hd: memo._0,
                            tl: {
                              hd: res._0,
                              tl: /* [] */0
                            }
                          })
                    };
            } else {
              return memo;
            }
          } else if (res.TAG) {
            return res;
          } else {
            return {
                    TAG: /* Ok */0,
                    _0: undefined
                  };
          }
        }), {
        TAG: /* Ok */0,
        _0: undefined
      }, Tablecloth.List.map((function (param) {
              var id = param[0];
              return mapError((function (param) {
                            return Tablecloth.List.map((function (err) {
                                          return [
                                                  id,
                                                  err
                                                ];
                                        }), param);
                          }), Curry._2(param[1], (function (op) {
                                return Curry._1(handleUndo, [
                                            id,
                                            op
                                          ]);
                              }), Tablecloth.List.filterMap((function (param) {
                                    if (id === param[0]) {
                                      return Caml_option.some(param[1]);
                                    }
                                    
                                  }), ops)));
            }), applyFns.contents));
  callChangeListeners(ops);
  return res;
}

var Undo = UndoRedo$Credt.Make({
      apply: baseApply
    });

function apply(id, ops) {
  return Curry._1(Undo.apply, Tablecloth.List.map((function (op) {
                    return [
                            id,
                            op
                          ];
                  }), ops));
}

var transaction = {
  contents: /* [] */0
};

var transactionRollbacks = {
  contents: /* [] */0
};

function addToTransaction(id, ops, rollback) {
  transaction.contents = Tablecloth.List.concat({
        hd: transaction.contents,
        tl: {
          hd: Tablecloth.List.map((function (op) {
                  return [
                          id,
                          op
                        ];
                }), ops),
          tl: /* [] */0
        }
      });
  transactionRollbacks.contents = {
    hd: rollback,
    tl: transactionRollbacks.contents
  };
  
}

function commitTransaction(allowErrorsOpt, param) {
  var allowErrors = allowErrorsOpt !== undefined ? allowErrorsOpt : false;
  var match = Curry._2(Undo.applyTransaction, allowErrors, transaction.contents);
  var res;
  if (allowErrors) {
    res = match;
  } else {
    if (match.TAG) {
      Tablecloth.List.iter((function (f) {
              return Curry._1(f, undefined);
            }), transactionRollbacks.contents);
      transactionRollbacks.contents = /* [] */0;
      transaction.contents = /* [] */0;
      return match;
    }
    res = match;
  }
  transactionRollbacks.contents = /* [] */0;
  transaction.contents = /* [] */0;
  return res;
}

function __reset__(param) {
  transaction.contents = /* [] */0;
  changeListeners.contents = /* [] */0;
  transactionRollbacks.contents = /* [] */0;
  return Curry._1(Undo.__reset__, undefined);
}

var undo = Undo.undo;

var canUndo = Undo.canUndo;

var getUndoHistory = Undo.getUndoHistory;

var redo = Undo.redo;

var canRedo = Undo.canRedo;

var getRedoHistory = Undo.getRedoHistory;

export {
  toOp ,
  applyFns ,
  register ,
  mapError ,
  changeListeners ,
  addChangeListener ,
  removeChangeListener ,
  callChangeListeners ,
  baseApply ,
  Undo ,
  apply ,
  transaction ,
  transactionRollbacks ,
  addToTransaction ,
  commitTransaction ,
  undo ,
  canUndo ,
  getUndoHistory ,
  redo ,
  canRedo ,
  getRedoHistory ,
  __reset__ ,
  
}
/* Undo Not a pure module */
