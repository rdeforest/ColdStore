import cold

compiler=cold.freon()
testmethod=compiler.compile(["arg a, b;return [a+b,a[b]];"]);

obj = cold.object()
obj.testmethod = testmethod

cold.names['obj']=obj

test = compiler.compile(["return $obj.testmethod(4,5);"]);
print test()


