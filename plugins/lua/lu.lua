unexpected_condition = false
function foo ()
	--if unexpected_condition then error() end
	error("1")
	--print(a[i])    -- potential error: `a' may not be a table
end

if pcall(foo) then
	print("no errors")
else
	print("errors")
end
