// Fill out your copyright notice in the Description page of Project Settings.


#include "FlibSourceControlHelper.h"
#include "GitSourceControlUtils.h"

FString UFlibSourceControlHelper::GetGitBinary()
{
	return GitSourceControlUtils::FindGitBinaryPath();
}

bool UFlibSourceControlHelper::RunGitCommandWithFiles(const FString& InCommand, const FString& InPathToGitBinary, const FString& InRepositoryRoot, const TArray<FString>& InParameters, const TArray<FString>& InFiles, TArray<FString>& OutResults, TArray<FString>& OutErrorMessages)
{
	return GitSourceControlUtils::RunCommand(InCommand, InPathToGitBinary, InRepositoryRoot, InParameters, InFiles, OutResults, OutErrorMessages);
}

bool UFlibSourceControlHelper::RunGitCommand(const FString& InCommand, const FString& InPathToGitBinary, const FString& InRepositoryRoot, const TArray<FString>& InParameters, TArray<FString>& OutResults, TArray<FString>& OutErrorMessages)
{
	return UFlibSourceControlHelper::RunGitCommandWithFiles(InCommand, InPathToGitBinary, InRepositoryRoot, InParameters, TArray<FString>{}, OutResults, OutErrorMessages);
}

bool UFlibSourceControlHelper::DiffVersion(const FString& InGitBinaey, const FString& InRepoRoot, const FString& InBeginCommitHash, const FString& InEndCommitHash, TArray<FString>& OutResault, TArray<FString>& OutErrorMessages)
{
	TArray<FString> DiffParams{
		InBeginCommitHash + FString(TEXT("...")) + InEndCommitHash,
		TEXT("--name-only"),
		TEXT("-r")
	};
	return UFlibSourceControlHelper::RunGitCommand(FString(TEXT("diff")), InGitBinaey, InRepoRoot, DiffParams, OutResault, OutErrorMessages);
}

bool UFlibSourceControlHelper::DiffVersionByGlobalGit(const FString& InRepoRoot, const FString& InBeginCommitHash,
	const FString& InEndCommitHash, TArray<FString>& OutResault)
{
	TArray<FString> OutErrorMessages;
	return DiffVersion(TEXT("git.exe"),InRepoRoot,InBeginCommitHash,InEndCommitHash,OutResault,OutErrorMessages);
}

bool UFlibSourceControlHelper::GitStatus(const FString& InGitBinaey, const FString& InRepoRoot,
	TArray<FString>& OutResault, const FString& DiffFilter)
{
	TArray<FString> Result;
	TArray<FString> OutErrorMessages;
	TArray<FString> Params{
		TEXT("--short")
	};
	bool bRunStatus = UFlibSourceControlHelper::RunGitCommand(FString(TEXT("status")), InGitBinaey, InRepoRoot, Params, Result, OutErrorMessages);
	for(auto& ChangedFile:Result)
	{
		ChangedFile.RemoveAt(0,3);
		OutResault.Add(ChangedFile);
	}
	return bRunStatus;
}

bool UFlibSourceControlHelper::GitStatusByGlobalGit(const FString& InRepoRoot, TArray<FString>& OutResault,
                                                    const FString& DiffFilter)
{
	return UFlibSourceControlHelper::GitStatus(TEXT("git"),InRepoRoot,OutResault,DiffFilter);
}

bool UFlibSourceControlHelper::GetFileLastCommitByGlobalGit(const FString& InRepositoryRoot, const FString& InFile,
                                                            FGitSourceControlRevisionData& OutHistory)
{
	TArray<FString> OutErrorMessages;
	return GetFileLastCommit(TEXT("git.exe"),InRepositoryRoot,InFile,false,OutErrorMessages,OutHistory);
}

bool UFlibSourceControlHelper::GitLog(const FString& InGitBinaey, const FString& InRepoRoot, TArray<FGitCommitInfo>& OutCommitInfo)
{
	TArray<FString> Params{
		TEXT("--pretty=format:\"%h;-;%s;-;%cd;-;%an\"")
	};
	TArray<FString> OutResault;
	TArray<FString> ErrorMessage;
	bool runResault = UFlibSourceControlHelper::RunGitCommand(FString(TEXT("log")), InGitBinaey, InRepoRoot, Params, OutResault, ErrorMessage);
	for (auto& Item : OutResault)
	{
		TArray<FString> ItemBreaker;
		Item.ParseIntoArray(ItemBreaker, TEXT(";-;"), true);
		OutCommitInfo.Add(FGitCommitInfo{ ItemBreaker[0],ItemBreaker[1], ItemBreaker[2], ItemBreaker[3] });
	}

	return runResault;
}

bool UFlibSourceControlHelper::GetBranchName(const FString& InPathToGitBinary, const FString& InRepositoryRoot, FString& OutBranchName)
{
	return GitSourceControlUtils::GetBranchName(InPathToGitBinary, InRepositoryRoot, OutBranchName);
}

bool UFlibSourceControlHelper::GetRemoteUrl(const FString& InPathToGitBinary, const FString& InRepositoryRoot, FString& OutRemoteUrl)
{
	return GitSourceControlUtils::GetRemoteUrl(InPathToGitBinary, InRepositoryRoot, OutRemoteUrl);
}

bool UFlibSourceControlHelper::GetConfigUserName(const FString& InPathToGitBinary, FString& OutUserName)
{
	bool bStatus = false;
	
	FString GitLocalUsername;
	bool bGitUsernameCmd = FParse::Value(FCommandLine::Get(), TEXT("-gitusername="), GitLocalUsername);
	if (bGitUsernameCmd)
	{
		OutUserName = GitLocalUsername;
	}
	else
	{
		const TArray<FString> Params = {TEXT("user.name")};
		TArray<FString> Result,OutErrorMessages;
		bool bRunStatus = UFlibSourceControlHelper::RunGitCommand(FString(TEXT("config")), InPathToGitBinary, TEXT(""), Params, Result, OutErrorMessages);
		if(bRunStatus && Result.Num())
		{
			OutUserName = Result[0];
			bStatus = true;
		}
	}
	return bStatus;
}

bool UFlibSourceControlHelper::GetConfigUserEmail(const FString& InPathToGitBinary, FString& OutUserEmail)
{
	bool bStatus = false;
	const TArray<FString> Params = {TEXT("user.email")};
	TArray<FString> Result,OutErrorMessages;
	bool bRunStatus = UFlibSourceControlHelper::RunGitCommand(FString(TEXT("config")), InPathToGitBinary, TEXT(""), Params, Result, OutErrorMessages);
	if(bRunStatus && Result.Num())
	{
		OutUserEmail = Result[0];
		bStatus = true;
	}
	return bStatus;
}

bool UFlibSourceControlHelper::FindRootDirectory(const FString& InPath, FString& OutRepositoryRoot)
{
	return GitSourceControlUtils::FindRootDirectory(InPath,OutRepositoryRoot);
}

EGitFileStatus UFlibSourceControlHelper::ParseFileStatus(const FString& FileStatusStr)
{
	EGitFileStatus result = EGitFileStatus::NoEdit;
	if(FileStatusStr.StartsWith(TEXT(" M ")))
	{
		result = EGitFileStatus::Modify;
	}
	if(FileStatusStr.StartsWith(TEXT(" A ")))
	{
		result = EGitFileStatus::Added;
	}
	if(FileStatusStr.StartsWith(TEXT(" D ")))
	{
		result = EGitFileStatus::Deleted;
	}
	if(FileStatusStr.StartsWith(TEXT(" R ")))
	{
		result = EGitFileStatus::Renamed;
	}
	if(FileStatusStr.StartsWith(TEXT(" C ")))
	{
		result = EGitFileStatus::Copoed;
	}
	if(FileStatusStr.StartsWith(TEXT(" U ")))
	{
		result = EGitFileStatus::Unmerged;
	}
	if(FileStatusStr.StartsWith(TEXT("?? ")))
	{
		result = EGitFileStatus::Untracked;
	}
	return result;
}

EGitFileStatus UFlibSourceControlHelper::GetFileStatus(const FString& InGitBinaey, const FString& InRepoRoot,
                                                       const FString InFile)
{
	EGitFileStatus StatusResult = EGitFileStatus::NoEdit;
	TArray<FString> Result;
	TArray<FString> OutErrorMessages;
	TArray<FString> Params{
		InFile,
		TEXT("--short")
	};
	bool bRunStatus = UFlibSourceControlHelper::RunGitCommand(FString(TEXT("status")), InGitBinaey, InRepoRoot, Params, Result, OutErrorMessages);
	for(auto& ChangedFile:Result)
	{
		StatusResult = ParseFileStatus(ChangedFile);
	}
	return StatusResult;
}

bool UFlibSourceControlHelper::RunGetHistory(const FString& InPathToGitBinary, const FString& InRepositoryRoot, const FString& InFile, bool bMergeConflict, TArray<FString>& OutErrorMessages, TArray<FGitSourceControlRevisionData>& OutHistory, int32 InHistoryDepth)
{
	TGitSourceControlHistory local_resault;
	bool bRunState = GitSourceControlUtils::RunGetHistory(InPathToGitBinary,InRepositoryRoot,InFile,bMergeConflict,OutErrorMessages,local_resault, InHistoryDepth);

	for (const auto& Item : local_resault)
	{
		FGitSourceControlRevisionData PureData;
		{
			PureData.Filename = Item.Get().Filename;
			PureData.CommitId = Item.Get().CommitId;
			PureData.ShortCommitId = Item.Get().ShortCommitId;
			PureData.CommitIdNumber = Item.Get().CommitIdNumber;
			PureData.RevisionNumber = Item.Get().RevisionNumber;
			PureData.FileHash = Item.Get().FileHash;
			PureData.Description = Item.Get().Description;
			PureData.UserName = Item.Get().UserName;
			PureData.Action = Item.Get().Action;
			PureData.Date = Item.Get().Date.ToString();
			PureData.FileSize = Item.Get().FileSize;
		}
		// Item.Get().GetRevisionData(CurrentRevisionData);
		OutHistory.Add(PureData);
	}

	return bRunState;
}

bool UFlibSourceControlHelper::GetFileLastCommit(const FString& InPathToGitBinary, const FString& InRepositoryRoot, const FString& InFile, bool bMergeConflict, TArray<FString>& OutErrorMessages, FGitSourceControlRevisionData& OutHistory)
{
	TArray<FGitSourceControlRevisionData> result;
	bool runState = RunGetHistory(InPathToGitBinary, InRepositoryRoot, InFile, bMergeConflict, OutErrorMessages, result, 1);

	if (result.Num() > 0)
	{
		runState = runState && true;
		OutHistory = result[0];
	}

	return runState;
}