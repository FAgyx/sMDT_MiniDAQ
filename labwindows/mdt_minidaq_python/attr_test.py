
class B:
    b1 = 'b1'
    b2 = 'b2'

class A:
    a1 = B()
    a2 = B()

    # a1 = self.f1()
    # a2 = self.f1()
    def printself(self):
        for i in range(2):
           print(getattr(getattr(self, 'a' + str(i + 1)),'b1'))

inst = A()
inst.printself()




# print(inst.a1.b1)
