using System;
using System.IO;
using System.Text;
using System.Threading.Tasks;

//D:\reposC++\PP\lw7\lw7RewriteApp\input.txt

class Program
{
   

    static async Task<string> ReadFileAsync(string filePath)
    {
        using (StreamReader reader = new StreamReader(filePath))
        {
            return await reader.ReadToEndAsync();
        }
    }

    static string RemoveChars(string content, string charsToRemove)
    {
        StringBuilder sb = new StringBuilder();

        foreach (char c in content)
        {
            if (!charsToRemove.Contains(c))
            {
                sb.Append(c);
            }
        }

        return sb.ToString();
    }

    static async Task WriteFileAsync(string filePath, string content)
    {
        using (StreamWriter writer = new StreamWriter(filePath, false))
        {
            await writer.WriteAsync(content);
        }
    }

    static async Task Main(string[] args)
    {
        Console.Write("Enter path to file: ");
        string filePath = Console.ReadLine();

        if (!File.Exists(filePath))
        {
            Console.WriteLine("Error opening file.");
            return;
        }

        Console.Write("Enter symbols to delete: ");
        string charsToRemove = Console.ReadLine();

        string fileContent = await ReadFileAsync(filePath);
        string newContent = RemoveChars(fileContent, charsToRemove);

        await WriteFileAsync(filePath, newContent);
        Console.WriteLine("Changes saved!");
    }
}