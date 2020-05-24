


function now(param) {
  return 0;
}

var lastId = {
  contents: 0
};

function idOfString(prim) {
  return prim;
}

function stringOfId(prim) {
  return prim;
}

function makeId(param) {
  var id = lastId.contents + 1 | 0;
  lastId.contents = id;
  return String(id);
}

export {
  now ,
  lastId ,
  idOfString ,
  stringOfId ,
  makeId ,
  
}
/* No side effect */
