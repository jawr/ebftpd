user = ebftpd.User.load(0)

print(user.name)
ret, msg = user:rename("default")

print(ret, msg)

created, msg = assert(ebftpd.User.create("1meow", "meow", 0, user))
print(created.name)

