fun treemap f node(f(p),glist) = node(p,map(treemap f) glist);
datatype 'a gametree = node of 'a * 'a gametree list;

fun prune 0 node(p,glist) = node(p,nil)
    | prune n node(p,glist) = node(p,map(prune(n-1)) glist)
    
fun treemap f node(f(p),glist) = node(p,map(treemap f) glist);