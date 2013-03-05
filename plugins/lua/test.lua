print("hello")
user = ebftpd.User.load(0)

print(user.name)
ret, msg = user:rename("default")

print(ret, msg)

created, msg = ebftpd.User.create("1meow", "meow", 0, user)

