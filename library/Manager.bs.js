

import * as Block from "bs-platform/lib/es6/block.js";
import * as Curry from "bs-platform/lib/es6/curry.js";
import * as UndoRedo from "./UndoRedo.bs.js";
import * as Tablecloth from "tablecloth-bucklescript/bucklescript/src/tablecloth.bs.js";
import * as Caml_option from "bs-platform/lib/es6/caml_option.js";

function toOp(prim) {
  return prim;
}

var applyFns = {
  contents: /* [] */0
};

function register(id, apply) {
  applyFns.contents = /* :: */[
    /* tuple */[
      id,
      apply
    ],
    applyFns.contents
  ];
  return /* () */0;
}

function mapError(fn, a) {
  if (a.tag) {
    return /* Error */Block.__(1, [Curry._1(fn, a[0])]);
  } else {
    return a;
  }
}

var changeListeners = {
  contents: /* [] */0
};

function addChangeListener(fn) {
  changeListeners.contents = /* :: */[
    fn,
    changeListeners.contents
  ];
  return /* () */0;
}

function removeChangeListener(fn) {
  changeListeners.contents = Tablecloth.List.filter((function (changeFn) {
          return changeFn !== fn;
        }), changeListeners.contents);
  return /* () */0;
}

function callChangeListeners(ops) {
  return Tablecloth.List.iter((function (fn) {
                return Curry._1(fn, ops);
              }), changeListeners.contents);
}

function baseApply(handleUndo, ops) {
  var res = Tablecloth.List.fold_left((function (res, memo) {
          if (memo.tag) {
            if (res.tag) {
              return /* Error */Block.__(1, [Tablecloth.List.concat(/* :: */[
                              memo[0],
                              /* :: */[
                                res[0],
                                /* [] */0
                              ]
                            ])]);
            } else {
              return memo;
            }
          } else if (res.tag) {
            return res;
          } else {
            return /* Ok */Block.__(0, [/* () */0]);
          }
        }), /* Ok */Block.__(0, [/* () */0]), Tablecloth.List.map((function (param) {
              var id = param[0];
              return mapError((function (param) {
                            return Tablecloth.List.map((function (err) {
                                          return /* tuple */[
                                                  id,
                                                  err
                                                ];
                                        }), param);
                          }), Curry._2(param[1], (function (op) {
                                return Curry._1(handleUndo, /* tuple */[
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

var Undo = UndoRedo.Make({
      apply: baseApply
    });

function apply(id, ops) {
  return Curry._1(Undo.apply, Tablecloth.List.map((function (op) {
                    return /* tuple */[
                            id,
                            op
                          ];
                  }), ops));
}

function applyTransaction(id, ops) {
  return Curry._1(Undo.applyTransaction, Tablecloth.List.map((function (op) {
                    return /* tuple */[
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
  transaction.contents = Tablecloth.List.concat(/* :: */[
        transaction.contents,
        /* :: */[
          Tablecloth.List.map((function (op) {
                  return /* tuple */[
                          id,
                          op
                        ];
                }), ops),
          /* [] */0
        ]
      ]);
  transactionRollbacks.contents = /* :: */[
    rollback,
    transactionRollbacks.contents
  ];
  return /* () */0;
}

function commitTransaction(param) {
  var e = Curry._1(Undo.applyTransaction, transaction.contents);
  if (e.tag) {
    Tablecloth.List.iter((function (f) {
            return Curry._1(f, /* () */0);
          }), transactionRollbacks.contents);
    transactionRollbacks.contents = /* [] */0;
    transaction.contents = /* [] */0;
    return e;
  } else {
    transactionRollbacks.contents = /* [] */0;
    transaction.contents = /* [] */0;
    return /* Ok */Block.__(0, [/* () */0]);
  }
}

function __reset__(param) {
  transaction.contents = /* [] */0;
  changeListeners.contents = /* [] */0;
  transactionRollbacks.contents = /* [] */0;
  return Curry._1(Undo.__reset__, /* () */0);
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
  applyTransaction ,
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
