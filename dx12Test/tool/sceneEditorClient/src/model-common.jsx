/**
 *  MVCのモデルに当たるクラスの共通処理をまとめたクラス
 */
class ModelBase
{
  constructor() {
    this.list = [];
  }
  add(obj) {
    this.list.push(obj);
  }
  delete(name) {
    let index = this.list.findIndex((element, index, array) => {
      return name == element.name;
    });
    if(index < this.list.length) {
      this.list.splice(index, 1);
    }
  }
  find(name) {
    return this.list.find((element, index, array) => {
      return name == element.name;
    });
  }
  traverse(out, input) {
    for(let key in input) {
      if(!out[key]) continue;

      if (typeof input[key] == 'object') {
        if(Array.isArray(input[key])) {
          out[key] = input[key];
        }else {
          this.traverse(out[key], input[key]);
        }
      } else {
        out[key] = input[key];
      }
    }
  }
  update(name, updateData) {
    let obj = this.find(name);
    if(!obj) return;
    this.traverse(obj, updateData);
  }
}

module.exports = {
  Class: ModelBase,
}
