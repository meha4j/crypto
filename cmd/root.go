package cmd

import (
	"fmt"
	"os"

	"github.com/meha4j/crypto/cmd/rot"
	"github.com/spf13/cobra"
)

var rootCmd = &cobra.Command{
	Use:   "crypto",
	Short: "Crypto is a collection of cryptographic algorithms.",
}

var versionCmd = &cobra.Command{
	Use:   "version",
	Short: "Print the version number of Crypto.",
	Run: func(cmd *cobra.Command, args []string) {
		fmt.Println("Version: 1.0")
	},
}

func init() {
	rootCmd.AddCommand(versionCmd)
	rootCmd.AddCommand(rot.RotCmd)
}

func Execute() {
	if err := rootCmd.Execute(); err != nil {
		fmt.Fprintln(os.Stderr, err)
		os.Exit(1)
	}
}
