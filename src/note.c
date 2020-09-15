#include <stdio.h>
#include <wordexp.h>
#include <ctype.h>
#include <sys/stat.h>
#include <unistd.h>
// The below are Liszt .h files
#include "note.h"
#include "helper.h"

#define MAX_LENGTH 256


void mergeNotes(char* args[], int numArgs) {
	char firstNote[MAX_LENGTH];
	char secondNote[MAX_LENGTH];
	int result = parseBinaryArgs(firstNote, secondNote, args, numArgs);
	if (result == -1) return;

	char* dirName = "main";
	char firstPath[MAX_LENGTH];
	char secondPath[MAX_LENGTH];
	getNotePath(dirName, firstNote, firstPath);
	getNotePath(dirName, secondNote, secondPath);

	// confirm that notes actually exist with the given name
	struct stat st = {0};
	if (stat(firstPath, &st) == -1) {
		printf("There is no note called '%s'. Please try again.\n", firstNote);
		return;
	}
	if (stat(secondPath, &st) == -1) {
		printf("There is no note called '%s'. Please try again.\n", secondNote);
		return;
	}

	// prevent the user from merging into or from default
	result = checkDefault(firstNote);
	if (result == -1) return;

	result = checkDefault(secondNote);
	if (result == -1) return;
	
	// append the first note to the second
	FILE* source, * target;
	source = fopen(firstPath, "r");
	target = fopen(secondPath, "a");
	char filechar;
	while ((filechar = fgetc(source)) != EOF) {
		fputc(filechar, target);
	}
	fclose(source);
	fclose(target);

	remove(firstPath);

	// change to default if merging the current note	
	char currentNote[MAX_LENGTH];
	getCurrentNoteName(currentNote);
	if (strcmp(currentNote, firstNote) == 0) {
		setToDefault();
	}
	
	printf("Successfully merged '%s' into '%s'\n", firstNote, secondNote);


}


void duplicateNote(char* args[], int numArgs) {
	char existingNote[MAX_LENGTH];
	char newNote[MAX_LENGTH];
	int result = parseBinaryArgs(existingNote, newNote, args, numArgs);
	if (result == -1) return;

	char* dirName = "main";
	char existingPath[MAX_LENGTH];
	char newPath[MAX_LENGTH];
	getNotePath(dirName, existingNote, existingPath);
	getNotePath(dirName, newNote, newPath);

	// confirm that a note actually exists with the given name
	struct stat st = {0};
	if (stat(existingPath, &st) == -1) {
		printf("There is no note called '%s'. Please try again.\n", existingNote);
		return;
	}

	// prevent the user from duplicating default
	result = checkDefault(existingNote);
	if (result == -1) return;
	
	result = makeNote(newPath);
	if (result == -1) {
		printf("Could not duplicate '%s'. Please try again.\n", existingNote);
		return;
	}
	copyFile(existingPath, newPath);
	printf("Successfully duplicated '%s' as '%s'\n", existingNote, newNote);
}


void importNote(char* args[], int numArgs) {
	char import[MAX_LENGTH];
	char note[MAX_LENGTH];
	int result = parseBinaryArgs(import, note, args, numArgs);
	struct stat st = {0};
	if (result == -1) {
		return;
	}

	// stop the user from even trying to import a note as 'default'
	result = checkDefault(import);
	if (result == -1) return;

	if (stat(import, &st) == -1) {
		printf("Sorry. \033[1mLiszt\033[0m can't find the file you are trying to import\n");
		return;
	}
	char* extension = strrchr(import, '.');
	if (!extension) {
		printf("Sorry. \033[1mLiszt\033[0m can't import the type of file you are trying to import\n");
		printf("\033[1mLiszt\033[0m only understands '.txt' files at the moment.\n");
	}
	if (strcmp(extension, ".txt") != 0) {
		printf("Sorry. \033[1mLiszt\033[0m can't import the type of file you are trying to import\n");
		printf("\033[1mLiszt\033[0m only understands '.txt' files at the moment.\n");
		return;
	}
	char* dirName = "main";	
	char fullNotePath[MAX_LENGTH];
	getNotePath(dirName, note, fullNotePath);

	result = makeNote(fullNotePath);
	if (result == 0) {
		copyFile(import, fullNotePath);	
		printf("Successfully imported '%s' as '%s'\n", import, note);
	}
}


void exportNote(char* args[], int numArgs) {
	char note[MAX_LENGTH];
	char toExport[MAX_LENGTH];
	int result = parseBinaryArgs(note, toExport, args, numArgs);

	if (result == -1) return;

	char notePath[MAX_LENGTH];
	char* dirName = "main";
	getNotePath(dirName, note, notePath);
	
	struct stat st = {0};
	if (stat(notePath, &st) == -1) {
		printf("Sorry. \033[1mLiszt\033[0m can't find the note you are trying to export: '%s'. Did you spell the name of the note correctly?\n", note);
		return;
	}

	// prevent the user from exporting default
	result = checkDefault(note);
	if (result == -1) return;
	
	strcat(toExport, ".txt");

	if (stat(toExport, &st) == 0) {
		printf("Sorry. A file with this name already exists in the directory to which you are trying to add. Please pick a different name\n");
		return;
	}

	copyFile(notePath, toExport);
	printf("Successfully exported '%s' as '%s'\n", note, toExport);
}


void archiveCurrent() {
	char currentNotePath[MAX_LENGTH];
	char currentNoteName[MAX_LENGTH];
	getCurrentNote(currentNotePath, currentNoteName);

	// prevent the user from archiving default
	int result = checkDefault(currentNoteName);
	if (result == -1) return;
	
	char* dirName = "archive";
	char newPath[MAX_LENGTH];
	getNotePath(dirName, currentNoteName, newPath);

	rename(currentNotePath, newPath);

	// switch current note to default
	setToDefault();

	printf("Archived '%s'\n", currentNoteName);
}


void archiveNote(char* args[], int numArgs) {
	char currentNotePath[MAX_LENGTH];
	char currentNoteName[MAX_LENGTH];
	getCurrentNote(currentNotePath, currentNoteName);

	char noteToArchive[MAX_LENGTH];
	parseUnaryArgs(noteToArchive, args, numArgs);
	char* dirName = "main";
	char notePath[MAX_LENGTH];
	getNotePath(dirName, noteToArchive, notePath);

	struct stat st = {0};	

	if (stat(notePath, &st) == -1) {
		printf("The note you are trying to archive does not exist. Please try again.\n");
		return;
	}

	// prevent the user from archiving default
	int result = checkDefault(noteToArchive);
	if (result == -1) return;

	dirName = "archive";
	char newPath[MAX_LENGTH];
	getNotePath(dirName, noteToArchive, newPath);

	if (stat(newPath, &st) != -1) {
		printf("You already have an archived note named '%s'. Please try again. (hint: rename something)\n", noteToArchive);
		return;
	}

	rename(notePath, newPath);

	if (strcmp(currentNoteName, noteToArchive) == 0) {
		setToDefault();
	}

	printf("Archived '%s'\n", noteToArchive);
	
}


void unArchiveNote(char* args[], int numArgs) {
	char noteToUnArchive[MAX_LENGTH];
	parseUnaryArgs(noteToUnArchive, args, numArgs);
	char* dirName = "archive";
	char notePath[MAX_LENGTH];
	getNotePath(dirName, noteToUnArchive, notePath);

	struct stat st = {0};	

	if (stat(notePath, &st) == -1) {
		printf("The note you are trying to un-archive does not exist. Please try again.\n");
		return;
	}

	dirName = "main";
	char newPath[MAX_LENGTH];
	getNotePath(dirName, noteToUnArchive, newPath);

	if (stat(newPath, &st) != -1) {
		printf("You already have a note named '%s'. Please try again. (hint: rename something)\n", noteToUnArchive);
		return;
	}

	rename(notePath, newPath);

	printf("Un-archived '%s'\n", noteToUnArchive);
}


void addNote(char* args[], int numArgs) {
	char note[MAX_LENGTH];
	char notePath[MAX_LENGTH];
	parseUnaryArgs(note, args, numArgs);

	// stop the user from even trying to name a note 'default'
	int result = checkDefault(note);
	if (result == -1) return;
	
	char* dirName = "main";
	getNotePath(dirName, note, notePath);

	result = makeNote(notePath);
	if (result == -1) {
		return;	
	}
	result = changeNoteHelper(note); 
	printf("Added new note '%s'\n", note);
}


void listNotes(char* directory) {
	wordexp_t dir;
	char path[20];
	strcpy(path, "~/.liszt/");
	strcat(path, directory);	
	wordexp(path, &dir, 0);

	char shortName[15];
	if (strcmp(directory, "main") == 0) {
		strcpy(shortName, " ");
	} else if (strcmp(directory, "archive") == 0) {
		strcpy(shortName, " archived ");
	}

	printDirectory(dir.we_wordv[0], shortName);

	wordfree(&dir);
}


void removeCurrent() {
	char currentNotePath[MAX_LENGTH];
	char currentNoteName[MAX_LENGTH];
	getCurrentNote(currentNotePath, currentNoteName);

	// prevent the user from archiving default
	int result = checkDefault(currentNoteName);
	if (result == -1) return;
	
	char prompt[MAX_LENGTH] = "Are you sure you want to remove '";
	strcat(prompt, currentNoteName);
	strcat(prompt, "'?\033[1m There is no going back (y/n): \033[0m");
 
	char decision[50];
	requestUserPermission(prompt, decision);
	if (strcmp(decision, "y") == 0) {
		remove(currentNotePath);
		// if the user is removing the current note
		setToDefault();

		printf("Sucessfully removed '%s'\n", currentNoteName);
	} else printf("Note removal aborted\n");
}


void removeNote(char* args[], int numArgs) {
	char currentNote[MAX_LENGTH];
	char currentNotePath[MAX_LENGTH];
	getCurrentNote(currentNotePath, currentNote);
	
	char dataFile[MAX_LENGTH];
	getDataFile(dataFile);

	char note[MAX_LENGTH];
	int result = parseUnaryArgs(note, args, numArgs);
	// prevent the user from deleting default
	result = checkDefault(note);
	if (result == -1) return;
	
	char prompt[MAX_LENGTH] = "Are you sure you want to remove '";
	strcat(prompt, note);
	strcat(prompt, "'?\033[1m There is no going back (y/n): \033[0m");
 
	char decision[50];
	requestUserPermission(prompt, decision);
	if (strcmp(decision, "y") == 0) {
		char* dirName = "main";
		char fullNotePath[MAX_LENGTH];
		getNotePath(dirName, note, fullNotePath);

		struct stat st = {0};
		if (stat(fullNotePath, &st) == -1) {
			printf("The note you are trying to remove does not exist. Please try again.\n");
			return;
		}
		remove(fullNotePath);
		// if the user is removing the current note
		if (strcmp(currentNote, note) == 0) {
			setToDefault();
		}
		
		 printf("Sucessfully removed '%s'\n", note);
	} else printf("Note removal aborted\n");

}


void clearNotes() {
	char currentNote[MAX_LENGTH];
	char currentNotePath[MAX_LENGTH];
	getCurrentNote(currentNotePath, currentNote);
	
	char dataFile[MAX_LENGTH];
	getDataFile(dataFile);

	char prompt[] = "Are you sure you want to clear all of your notes?\033[1m There is no going back (y/n): \033[0m";
	char decision[50];
	requestUserPermission(prompt, decision);
	if (strcmp(decision, "y") == 0) {
		wordexp_t mainDir;
		wordexp("~/.liszt/main", &mainDir, 0);
		char* notes[MAX_LENGTH];
		int numNotes = 0;
		readDirectory(mainDir.we_wordv[0], notes, &numNotes);
		if (numNotes == 1) {
			printf("You have no notes to clear!\n");
			wordfree(&mainDir);
			return;
		}

		for (int i = 0; i < numNotes; i++) {
			if (strcmp(notes[i], "default") == 0) continue;
			char* dirName = "main";
			char note[MAX_LENGTH];
			getNotePath(dirName, notes[i], note);
			remove(note);	
		}
		if (strcmp(currentNote, "default") != 0) {
			setToDefault();
		}
		wordfree(&mainDir);	
		printf("Cleared all user notes from Liszt cache\n");
	} else printf("Clearing of notes aborted\n");
}


void clearArchiveNotes() {
	char prompt[] = "Are you sure you want to clear all of your archived notes?\033[1m There is no going back (y/n): \033[0m";
	char decision[50];
	requestUserPermission(prompt, decision);
	if (strcmp(decision, "y") == 0) {
		wordexp_t archiveDir;
		wordexp("~/.liszt/archive", &archiveDir, 0);
		char* archives[MAX_LENGTH];
		int numNotes = 0;
		readDirectory(archiveDir.we_wordv[0], archives, &numNotes);
		if (!numNotes) {
			printf("You have no notes to clear!\n");
			wordfree(&archiveDir);
			return;
		}

		for (int i = 0; i < numNotes; i++) {
			char* dirName = "archive";
			char note[MAX_LENGTH];
			getNotePath(dirName, archives[i], note);
			remove(note);	
		}
		wordfree(&archiveDir);	
		printf("Cleared all archived user notes from Liszt cache\n");
	} else printf("Clearing of notes aborted\n");
}


int changeNoteHelper(char* note) {
	char dataFile[MAX_LENGTH];
	getDataFile(dataFile);
	char currentNotePath[MAX_LENGTH];
	char currentNoteName[MAX_LENGTH];
	getCurrentNote(currentNotePath, currentNoteName);

	// stop if the note to be changed to is the current note	
	if (strcmp(currentNoteName, note) == 0) {
		printf("The note you are trying to change to is already the current note.\n");
		return -1;
	}
	// stop if the note to be changed to is 'default'
	int result = checkDefault(note);
	if (result == -1) return -1;

	struct stat st = {0};
	
	char* dirName = "main";
	char notePath[MAX_LENGTH];
	getNotePath(dirName, note, notePath);

	if (stat(notePath, &st) == -1) {
		printf("Hmmm. The note you entered doesn't seem to exist. Please try again.\n");
		return -1;
	}	

	if (strcmp(currentNoteName, "default") == 0) {
		int numMemories = getFileSize(currentNotePath);
		if (numMemories > 0) {
			char newNote[MAX_LENGTH];
			printf("The current note must be named before changing notes. Please enter a name (ENTER to delete the current note): ");
			fgets(newNote, MAX_LENGTH, stdin);
			int length = strlen(newNote);
			if (length > 0) {
				newNote[length - 1] = '\0';
				char newNotePath[MAX_LENGTH];	
				getNotePath(dirName, newNote, newNotePath);

				result = makeNote(newNotePath);
				if (result == -1) return -1;
				copyFile(currentNotePath, newNotePath);
				// the following is for clearing 'default'
				FILE* toClear = fopen(currentNotePath, "w");
				fclose(toClear);
			}
		}
	}
	writeFilenameToDataFile(notePath);
	return 0;
}

 
void changeNote(char* args[], int numArgs) {
	char note[MAX_LENGTH];
	parseUnaryArgs(note, args, numArgs);
	int result = changeNoteHelper(note);
	if (result == 0) printf("Changed current note to '%s'\n", note); 
}


void renameNote(char* args[], int numArgs) {
	char oldName[MAX_LENGTH];
	char newName[MAX_LENGTH];
	int result = parseBinaryArgs(oldName, newName, args, numArgs);
	if (result == -1) return;
	
	char* dirName = "main";	
	char oldPath[MAX_LENGTH];
	char newPath[MAX_LENGTH];
	getNotePath(dirName, oldName, oldPath);
	getNotePath(dirName, newName, newPath);

	// confirm that the note to change actually exists
	struct stat st = {0};
	if (stat(oldPath, &st) == -1) {
		printf("The note you are trying to rename does not exist. Please try again.\n");
		return;
	}

	// prevent the user from renaming 'default'
	result = checkDefault(oldName);
	if (result == -1) return;

	rename(oldPath, newPath);

	char currentNotePath[MAX_LENGTH];
	char currentNoteName[MAX_LENGTH];
	getCurrentNote(currentNotePath, currentNoteName);

	if (strcmp(oldPath, currentNotePath) == 0) {
		writeFilenameToDataFile(newPath);
	}
	
	printf("Renamed '%s' to '%s'\n", oldName, newName);
}

