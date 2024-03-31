let n = 9;
let n1 = 0;
let n2 = 1;
let i = 0;
for i = 0; i < n; i = i + 1 {
    let tmp = n2;
    n2 = n2 + n1;
    n1 = tmp;
}
return n2;

// let a = 68;
// if a < 20 {
//     return a+1;
// } else {
//     return a-1;
// }

// let a = 1 + 69 / 12 * (62 - 17);

// fn addone ciao come stai {
//     let a = 2;
//     let b = 3;
//     let c = a + b;
// }
