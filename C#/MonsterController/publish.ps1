# On Windows, run this to publish ($args[0] is platform and $args[1] is true or false depending if you want to trim it)

if ($args.count -eq 2)
{
    dotnet publish -r $args[0] -c Release -p:PublishReadyToRun=true -p:PublishSingleFile=true -p:PublishTrimmed=$args[1] --self-contained true -p:IncludeNativeLibrariesForSelfExtract=true
}
elseif ($args.count -eq 1)
{
    dotnet publish -r $args[0] -c Release -p:PublishReadyToRun=true -p:PublishSingleFile=true -p:PublishTrimmed=true --self-contained true -p:IncludeNativeLibrariesForSelfExtract=true
}
elseif ($args.count -eq 0)
{
    dotnet publish -c Release -p:PublishReadyToRun=true -p:PublishSingleFile=true -p:PublishTrimmed=true --self-contained true -p:IncludeNativeLibrariesForSelfExtract=true
}
else
{
    Write-Output "incorrect arguments"
}