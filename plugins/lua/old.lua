ebftpd.test()



--[[
id1 = pcall(ebftpd.hook_event(ebftpd.Event.logged_in, function (client, arg1, arg2, arg3) print("hook test", client, arg1, arg2, arg3) end))
id2 = pcall(ebftpd.hook_event(ebftpd.Event.logged_in, function (client, arg1, arg2, arg3) print("look mum! multiple hooks..") end))

print(id1)
ebftpd.unhook_event(id1)

id3 = ebftpd.hook_command("pre", "Pre command", "*", function (client, cmd_line, args) print("command!", cmd_line) end)
print(id3)
id4 = ebftpd.hook_command("pre", "Pre command", "*", function (client, cmd_line, args) print("command!", cmd_line) end)
print(id4)
id5 = ebftpd.hook_command("help", "Pre command", "*", function (client, cmd_line, args) print("command!", cmd_line) end)
print(id5)

--[[for i, v in ipairs(class_names()) do 
	print(v)
	for k, v in pairs(class_info(v).methods) do print(k, v) end
	print(v)
end

user = ebftpd.User.load(0)

print(user.last_login, os.date("%x %X", user.last_login))
user:rename("w00t")
assert(user.name == "w00t")
user:rename("ebftpd")
assert(user.name == "ebftpd")

--num_masks = # user:ip_masks
user:add_ip_mask("gfkgjdfkljgldf")
--assert(# user:ip_masks == num_masks + 1)
user:del_ip_mask("gfkgjdfkljgldf")
--assert(# user:ip_masks == num_masks)
user:set_password("ebftpd")
assert(user:verify_password("ebftpd"))
user:set_password("password")
assert(user:verify_password("password"))
user:set_password("ebftpd")
assert(not user:verify_password("gdfgdfgdf"))

print(user.flags)
user:add_flags("234")
print(user.flags)
user:del_flags("234")
print(user.flags)

print(user.primary_gid, user.primary_group)
user.primary_gid = 1

print(user.primary_gid, user.primary_group)
user.primary_gid = 0

assert(user:has_gid(0))
assert(not user:has_gid(69))
user:add_gids({9,8,7})

print(#user.gadmin_gids)

for k, v in next, user.gadmin_gids, nil
do
  print("gad ", k, v)
end

user:set_gids({0})
for k, v in next, user.secondary_gids, nil
do
  print("2nd ", k, v)
end

print("!", os.date(user.created))

--user.expires = os.date()
print(user.expired)
print("***")
expires = user.expires
print("---")
if not expires then
  print("no expire")
else
  print(os.date(expires))
end

user = ebftpd.User.load(0)
user.expires = nil
user.expires = os.time()

ex = user.expires
print(os.date("%x %X", ex))

print(user:get_section_ratio("MP3"))
print(user:get_section_credits("MP3"))
print(user.default_ratio)


assert(not ebftpd.Group.load("xxx"))
group = ebftpd.Group.load("default")
assert(group)
print(group.name)
print(group.description)


print(ebftpd.Message.Status.unread)

msg = ebftpd.Message("ebftpd", ebftpd.name_to_uid("default"), "this is a message", os.time())

print(msg.status)
assert(msg.status == ebftpd.Message.Status.unread)
---]]