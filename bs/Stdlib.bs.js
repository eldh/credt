

import * as Belt_MapString from "bs-platform/lib/es6/belt_MapString.js";

function Make(C) {
  var add = function (id, t, data) {
    return Belt_MapString.set(data, id, t);
  };
  var remove = function (id, data) {
    return Belt_MapString.remove(data, id);
  };
  var iter = function (fn, data) {
    return Belt_MapString.forEach(data, fn);
  };
  var find = function (id, data) {
    return Belt_MapString.getExn(data, id);
  };
  return {
          isEmpty: Belt_MapString.isEmpty,
          has: Belt_MapString.has,
          cmpU: Belt_MapString.cmpU,
          cmp: Belt_MapString.cmp,
          eqU: Belt_MapString.eqU,
          eq: Belt_MapString.eq,
          findFirstByU: Belt_MapString.findFirstByU,
          findFirstBy: Belt_MapString.findFirstBy,
          forEachU: Belt_MapString.forEachU,
          forEach: Belt_MapString.forEach,
          reduceU: Belt_MapString.reduceU,
          reduce: Belt_MapString.reduce,
          everyU: Belt_MapString.everyU,
          every: Belt_MapString.every,
          someU: Belt_MapString.someU,
          some: Belt_MapString.some,
          size: Belt_MapString.size,
          toList: Belt_MapString.toList,
          toArray: Belt_MapString.toArray,
          fromArray: Belt_MapString.fromArray,
          keysToArray: Belt_MapString.keysToArray,
          valuesToArray: Belt_MapString.valuesToArray,
          minKey: Belt_MapString.minKey,
          minKeyUndefined: Belt_MapString.minKeyUndefined,
          maxKey: Belt_MapString.maxKey,
          maxKeyUndefined: Belt_MapString.maxKeyUndefined,
          minimum: Belt_MapString.minimum,
          minUndefined: Belt_MapString.minUndefined,
          maximum: Belt_MapString.maximum,
          maxUndefined: Belt_MapString.maxUndefined,
          get: Belt_MapString.get,
          getUndefined: Belt_MapString.getUndefined,
          getWithDefault: Belt_MapString.getWithDefault,
          getExn: Belt_MapString.getExn,
          checkInvariantInternal: Belt_MapString.checkInvariantInternal,
          removeMany: Belt_MapString.removeMany,
          set: Belt_MapString.set,
          updateU: Belt_MapString.updateU,
          update: Belt_MapString.update,
          mergeU: Belt_MapString.mergeU,
          merge: Belt_MapString.merge,
          mergeMany: Belt_MapString.mergeMany,
          keepU: Belt_MapString.keepU,
          keep: Belt_MapString.keep,
          partitionU: Belt_MapString.partitionU,
          partition: Belt_MapString.partition,
          split: Belt_MapString.split,
          mapU: Belt_MapString.mapU,
          map: Belt_MapString.map,
          mapWithKeyU: Belt_MapString.mapWithKeyU,
          mapWithKey: Belt_MapString.mapWithKey,
          empty: null,
          add: add,
          cardinal: Belt_MapString.size,
          remove: remove,
          iter: iter,
          to_seq: Belt_MapString.toList,
          find: find
        };
}

var $$Map = {
  Make: Make
};

function of_seq(a) {
  return a;
}

var List = {
  of_seq: of_seq
};

export {
  $$Map ,
  List ,
  
}
/* No side effect */
